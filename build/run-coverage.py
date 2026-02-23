#!/usr/bin/env python3
#==========================================================================
# ObTools: build/run-coverage.py
#
# Run gcov code coverage for a single library's test suite.
#
# Prerequisites: g++, gcov (both standard with build-essential)
# Google Test: auto-detected from system or built from source in /tmp
#
# Usage:
#   build/run-coverage.py <library-name> [options]
#
# Examples:
#   build/run-coverage.py xml          # Coverage for libs/xml
#   build/run-coverage.py json         # Coverage for libs/json
#   build/run-coverage.py xml -v       # Verbose: show uncovered lines
#   build/run-coverage.py xml -f       # Show per-function coverage
#
# The script will:
#   1. Build all core libraries via Make (if not already built)
#   2. Compile the library's .cc files with --coverage
#   3. Compile and link each test-*.cc file
#   4. Run all tests and collect coverage data
#   5. Report per-file line coverage
#
# Copyright (c) 2026 Paul Clark.
#==========================================================================

import argparse
import glob
import os
import re
import shutil
import subprocess
import sys


REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LIBS_DIR = os.path.join(REPO_ROOT, "libs")
BUILD_DIR = os.path.join(LIBS_DIR, "build")
COV_DIR = "/tmp/obtools-coverage"


# ── Google Test discovery ────────────────────────────────────────────────

def find_gtest():
    """Locate Google Test headers and libraries.

    Returns (include_dir, lib_dir) or exits on failure.
    """
    # System install
    if os.path.isfile("/usr/include/gtest/gtest.h"):
        for d in ("/usr/lib", "/usr/lib/x86_64-linux-gnu", "/usr/local/lib"):
            if os.path.isfile(os.path.join(d, "libgtest.a")):
                return "/usr/include", d

    # Pre-built in /tmp
    tmp_inc = "/tmp/googletest/googletest/include"
    tmp_lib = "/tmp/googletest/build/lib"
    if (os.path.isfile(os.path.join(tmp_inc, "gtest/gtest.h"))
            and os.path.isfile(os.path.join(tmp_lib, "libgtest.a"))):
        return tmp_inc, tmp_lib

    # Build from source
    print("Google Test not found. Building from source...")
    for tool in ("cmake", "git"):
        if shutil.which(tool) is None:
            sys.exit(f"Error: {tool} is required to build Google Test")

    if not os.path.isdir("/tmp/googletest"):
        subprocess.run(
            ["git", "clone", "--depth", "1", "--branch", "v1.14.0",
             "https://github.com/google/googletest.git"],
            cwd="/tmp", check=True,
        )
    subprocess.run(
        ["cmake", "-S", ".", "-B", "build", "-DCMAKE_BUILD_TYPE=Release"],
        cwd="/tmp/googletest", check=True,
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
    )
    nproc = os.cpu_count() or 1
    subprocess.run(
        ["cmake", "--build", "build", f"-j{nproc}"],
        cwd="/tmp/googletest", check=True,
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
    )
    return tmp_inc, tmp_lib


# ── Tupfile dependency resolution ────────────────────────────────────────

def parse_tupfile_var(path, var_name):
    """Extract a (possibly backslash-continued) variable assignment from a
    Tupfile.  Returns a list of whitespace-separated tokens."""
    if not os.path.isfile(path):
        return []
    with open(path) as f:
        text = f.read()

    # Match lines like:  VAR = val1 val2 \
    #                          val3
    pattern = rf"^{re.escape(var_name)}\s*=(.*?)(?:\n(?!\s)|\Z)"
    m = re.search(pattern, text, re.MULTILINE | re.DOTALL)
    if not m:
        return []
    body = m.group(1)
    # Remove backslash-newline continuations
    body = body.replace("\\\n", " ")
    return body.split()


def resolve_deps(lib, libs_dir, build_dir, visited=None, link_libs=None):
    """Recursively resolve transitive library dependencies by parsing
    Tupfiles, mirroring the Tuprules.lua logic.

    Returns an ordered list of .a file paths (deepest deps first).
    """
    if visited is None:
        visited = set()
    if link_libs is None:
        link_libs = []

    if lib in visited:
        return link_libs
    visited.add(lib)

    tupfile = os.path.join(libs_dir, lib, "Tupfile")

    deps = parse_tupfile_var(tupfile, "DEPENDS")
    deps += parse_tupfile_var(tupfile, "LINUX-DEPENDS")

    for dep in deps:
        # Skip external deps
        if dep.startswith("ext-"):
            continue
        # Strip ot- prefix to get directory name
        dep_lib = dep[3:] if dep.startswith("ot-") else dep

        # Recurse (depth-first)
        resolve_deps(dep_lib, libs_dir, build_dir, visited, link_libs)

        # Add .a file if the dep is a lib
        dep_tupfile = os.path.join(libs_dir, dep_lib, "Tupfile")
        dep_types = parse_tupfile_var(dep_tupfile, "TYPE")
        dep_type = dep_types[0] if dep_types else "lib"
        if dep_type == "lib":
            afile = os.path.join(build_dir, "lib", f"ot-{dep_lib}.a")
            if afile not in link_libs:
                link_libs.append(afile)

    return link_libs


# ── Compilation helpers ──────────────────────────────────────────────────

def run(cmd, **kwargs):
    """Run a command, returning CompletedProcess.  Prints stderr on failure."""
    return subprocess.run(cmd, **kwargs)


def build_libraries():
    """Build core libraries via Make."""
    print("--- Building libraries ---")
    nproc = os.cpu_count() or 1
    result = run(
        ["make", f"-j{nproc}", "core"],
        cwd=LIBS_DIR, capture_output=True, text=True,
    )
    # Show last few lines of output
    lines = (result.stdout + result.stderr).strip().splitlines()
    for line in lines[-3:]:
        print(f"  {line}")
    if result.returncode != 0:
        sys.exit("Error: library build failed")
    print()


def compile_sources(lib_dir, inc_dir, cov_dir):
    """Compile library .cc files with --coverage.

    Returns (src_files, obj_files).
    """
    print(f"--- Compiling sources with coverage ---")
    src_files = []
    obj_files = []

    for src in sorted(glob.glob(os.path.join(lib_dir, "*.cc"))):
        base = os.path.basename(src)
        if base.startswith("test-") or base.startswith("legacy-"):
            continue
        obj = os.path.join(cov_dir, base.replace(".cc", ".o"))
        result = run(
            ["g++", "--std=c++17", "-pedantic", "-Wall", "-Wextra", "-Werror",
             "-Wno-unknown-pragmas",
             "--coverage", "-fprofile-arcs", "-ftest-coverage",
             f"-I{inc_dir}", "-c", src, "-o", obj],
            capture_output=True, text=True,
        )
        if result.returncode != 0:
            print(f"  ERROR compiling {base}:")
            print(result.stderr)
            sys.exit(1)
        src_files.append(src)
        obj_files.append(obj)

    print(f"  Compiled {len(src_files)} source files")
    return src_files, obj_files


def build_tests(lib_dir, inc_dir, cov_dir, obj_files, gtest_inc, gtest_lib,
                link_libs):
    """Build test binaries.  Returns list of (binary_path, test_name)."""
    print()
    print("--- Building test binaries ---")
    test_bins = []

    for test_src in sorted(glob.glob(os.path.join(lib_dir, "test-*.cc"))):
        base = os.path.basename(test_src).replace(".cc", "")
        if base.startswith("legacy-"):
            continue

        bin_path = os.path.join(cov_dir, base)
        cmd = [
            "g++", "--std=c++17", "-pedantic", "-Wall", "-Wextra", "-Werror",
            "-Wno-unknown-pragmas", "--coverage",
            f"-I{inc_dir}", f"-I{gtest_inc}",
            test_src,
        ] + obj_files + [
            f"-L{gtest_lib}", f"-L{os.path.join(BUILD_DIR, 'lib')}",
            "-Wl,--start-group",
        ] + link_libs + [
            "-Wl,--end-group",
            "-lgtest", "-lpthread",
            "-o", bin_path,
        ]

        result = run(cmd, capture_output=True, text=True)
        if result.returncode == 0:
            test_bins.append((bin_path, base))
            print(f"  Built: {base}")
        else:
            print(f"  SKIP: {base} (link failed — may need extra deps)")

    if not test_bins:
        sys.exit("Error: no test binaries could be built")
    return test_bins


# ── Test execution ───────────────────────────────────────────────────────

def run_tests(test_bins):
    """Run all test binaries and report results.

    Returns (total_passed, total_tests).
    """
    print()
    print("--- Running tests ---")
    total_passed = 0
    total_tests = 0

    for bin_path, name in test_bins:
        result = run(
            [bin_path, "--gtest_brief=1"],
            capture_output=True, text=True,
        )
        output = result.stdout + result.stderr

        passed = 0
        failed = 0
        for line in output.splitlines():
            m = re.search(r"(\d+)\s+test", line)
            if "PASSED" in line and m:
                passed = int(m.group(1))
            elif "FAILED" in line and m:
                failed = int(m.group(1))

        total = passed + failed
        total_tests += total
        total_passed += passed

        if failed > 0:
            print(f"  {name}: {passed}/{total} PASSED ({failed} FAILED)")
            for line in output.splitlines():
                if "FAILED" in line:
                    print(f"    {line.strip()}")
        else:
            print(f"  {name}: {passed} passed")

    print()
    print(f"  Total: {total_passed} / {total_tests} tests passed")
    return total_passed, total_tests


# ── gcov coverage analysis ───────────────────────────────────────────────

def parse_gcov_file(gcov_path):
    """Parse a .gcov file.  Returns (hit, miss, uncovered_lines).

    uncovered_lines is a list of (lineno, code) tuples.

    Recognises GCOV_EXCL_LINE, GCOV_EXCL_START and GCOV_EXCL_STOP markers
    in source comments to exclude lines from the miss count (similar to how
    lcov handles LCOV_EXCL markers).
    """
    hit = 0
    miss = 0
    uncovered = []
    in_excl_block = False

    with open(gcov_path) as f:
        for line in f:
            parts = line.split(":", 2)
            if len(parts) < 3:
                continue
            count = parts[0].strip()
            lineno = parts[1].strip()
            code = parts[2].rstrip()

            # Track exclusion blocks
            if "GCOV_EXCL_START" in code:
                in_excl_block = True
            if "GCOV_EXCL_STOP" in code:
                in_excl_block = False
                continue

            # Skip excluded lines
            excluded = in_excl_block or "GCOV_EXCL_LINE" in code

            if count in ("-", ""):
                continue
            if count == "#####":
                if not excluded:
                    miss += 1
                    stripped = code.strip()
                    if stripped and not stripped.startswith("//"):
                        uncovered.append((lineno, stripped))
            elif count.replace("*", "").isdigit():
                n = int(count.replace("*", ""))
                if n > 0:
                    hit += 1

    return hit, miss, uncovered


def run_gcov(src_files, cov_dir):
    """Run gcov on all source files.  Returns dict mapping basename to
    parsed (hit, miss, uncovered) tuples."""
    for src in src_files:
        run(
            ["gcov", "-o", cov_dir, src],
            capture_output=True, text=True,
        )

    results = {}
    for src in src_files:
        base = os.path.basename(src)
        gcov_file = base + ".gcov"
        if os.path.isfile(gcov_file):
            results[base] = parse_gcov_file(gcov_file)
        else:
            results[base] = None
    return results


def report_coverage(lib_name, src_files, cov_results):
    """Print the coverage summary table."""
    print()
    print("=" * 64)
    print(f"CODE COVERAGE — libs/{lib_name}")
    print("=" * 64)
    print()
    print(f"  {'FILE':30s} {'HIT':>6s}  {'TOTAL':>6s}  {'COVERAGE':>8s}")
    print(f"  {'----':30s} {'---':>6s}  {'-----':>6s}  {'--------':>8s}")

    grand_hit = 0
    grand_total = 0

    for src in src_files:
        base = os.path.basename(src)
        data = cov_results.get(base)
        if data is None:
            print(f"  {base:30s}      -       -    no data")
            continue

        hit, miss, _ = data
        total = hit + miss
        grand_hit += hit
        grand_total += total

        if total > 0:
            pct = hit * 100.0 / total
            print(f"  {base:30s} {hit:6d}  {total:6d}    {pct:5.1f}%")
        else:
            print(f"  {base:30s}      -       -      n/a")

    print()
    if grand_total > 0:
        gpct = grand_hit * 100.0 / grand_total
        print(f"  {'TOTAL':30s} {grand_hit:6d}  {grand_total:6d}    {gpct:5.1f}%")
    else:
        print("  No coverage data collected.")
    print()


def report_uncovered(src_files, cov_results):
    """Print uncovered lines for each source file (-v mode)."""
    print("=" * 64)
    print("UNCOVERED LINES")
    print("=" * 64)

    for src in src_files:
        base = os.path.basename(src)
        data = cov_results.get(base)
        if data is None:
            continue
        _, _, uncovered = data
        if uncovered:
            print(f"\n--- {base} ({len(uncovered)} lines) ---")
            for lineno, code in uncovered:
                print(f"  {lineno:>4s}: {code[:72]}")
    print()


def report_functions(src_files, cov_dir):
    """Print per-function coverage (-f mode) using gcov -f and c++filt."""
    print("=" * 64)
    print("FUNCTION COVERAGE")
    print("=" * 64)

    has_cppfilt = shutil.which("c++filt") is not None

    for src in src_files:
        base = os.path.basename(src)
        print(f"\n--- {base} ---")

        result = run(
            ["gcov", "-f", "-o", cov_dir, src],
            capture_output=True, text=True,
        )

        # gcov -f output alternates:
        #   Function '<mangled>'
        #   Lines executed:NN.NN% of N
        lines = result.stdout.splitlines()
        i = 0
        while i < len(lines) - 1:
            func_line = lines[i]
            cov_line = lines[i + 1]

            m_func = re.match(r"Function '(.+)'", func_line)
            m_cov = re.search(r"([\d.]+%)", cov_line)

            if m_func and m_cov:
                mangled = m_func.group(1)
                pct = m_cov.group(1)

                # Only show ObTools functions
                if "_ZN" in mangled and "7ObTools" in mangled:
                    if has_cppfilt:
                        dfilt = run(
                            ["c++filt", mangled],
                            capture_output=True, text=True,
                        )
                        demangled = dfilt.stdout.strip()
                    else:
                        demangled = mangled

                    # Simplify common patterns
                    demangled = re.sub(
                        r"std::__cxx11::basic_string<char,"
                        r" std::char_traits<char>,"
                        r" std::allocator<char>\s*>",
                        "string", demangled)
                    demangled = re.sub(
                        r"ObTools::\w+::", "", demangled)
                    demangled = demangled.replace(" const", "")

                    print(f"  {demangled:60s} {pct}")
                i += 2
            else:
                i += 1
    print()


# ── Cleanup ──────────────────────────────────────────────────────────────

def cleanup_gcov_files():
    """Remove .gcov files from the working directory."""
    for f in glob.glob("*.gcov"):
        os.remove(f)


# ── Main ─────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description="Run gcov code coverage for an ObTools library's tests.")
    parser.add_argument("library", help="Library name (e.g. xml, json)")
    parser.add_argument("-v", "--verbose", action="store_true",
                        help="Show uncovered lines")
    parser.add_argument("-f", "--functions", action="store_true",
                        help="Show per-function coverage")
    args = parser.parse_args()

    lib_dir = os.path.join(LIBS_DIR, args.library)
    if not os.path.isdir(lib_dir):
        sys.exit(f"Error: library directory not found: {lib_dir}")

    inc_dir = os.path.join(BUILD_DIR, "include")

    # Locate Google Test
    gtest_inc, gtest_lib = find_gtest()

    # Resolve transitive dependencies
    link_libs = resolve_deps(args.library, LIBS_DIR, BUILD_DIR)

    # Build libraries
    build_libraries()

    # Prepare coverage directory
    if os.path.isdir(COV_DIR):
        shutil.rmtree(COV_DIR)
    os.makedirs(COV_DIR)

    # Compile sources with coverage
    src_files, obj_files = compile_sources(lib_dir, inc_dir, COV_DIR)

    # Build test binaries
    test_bins = build_tests(lib_dir, inc_dir, COV_DIR, obj_files,
                            gtest_inc, gtest_lib, link_libs)

    # Run tests
    run_tests(test_bins)

    # Generate coverage data
    cov_results = run_gcov(src_files, COV_DIR)

    # Report
    report_coverage(args.library, src_files, cov_results)

    if args.verbose:
        report_uncovered(src_files, cov_results)

    if args.functions:
        report_functions(src_files, COV_DIR)

    # Cleanup
    cleanup_gcov_files()

    print(f"Coverage data saved to: {COV_DIR}")
    print("Re-run with -v for uncovered lines, -f for function detail.")


if __name__ == "__main__":
    main()
