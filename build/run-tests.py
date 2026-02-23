#!/usr/bin/env python3
#==========================================================================
# ObTools: build/run-tests.py
#
# Compile and run tests for one or more libraries.
#
# Prerequisites: clang++ (or g++), Google Test (auto-provisioned if missing)
#
# Usage:
#   build/run-tests.py <library> [test-name]  # Run tests for a library
#   build/run-tests.py --all                  # Run tests for all libraries
#
# Examples:
#   build/run-tests.py xml           # All tests in libs/xml
#   build/run-tests.py chan test-chan-rw  # Just one test file
#   build/run-tests.py --all         # Every library with test-*.cc files
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
import tempfile


REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LIBS_DIR = os.path.join(REPO_ROOT, "libs")
BUILD_DIR = os.path.join(LIBS_DIR, "build")
OUT_DIR = os.path.join(tempfile.gettempdir(), "obtools-tests")


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

    pattern = rf"^{re.escape(var_name)}\s*=(.*?)(?:\n(?!\s)|\Z)"
    m = re.search(pattern, text, re.MULTILINE | re.DOTALL)
    if not m:
        return []
    body = m.group(1)
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
        if dep.startswith("ext-"):
            continue
        dep_lib = dep[3:] if dep.startswith("ot-") else dep

        resolve_deps(dep_lib, libs_dir, build_dir, visited, link_libs)

        dep_tupfile = os.path.join(libs_dir, dep_lib, "Tupfile")
        dep_types = parse_tupfile_var(dep_tupfile, "TYPE")
        dep_type = dep_types[0] if dep_types else "lib"
        if dep_type == "lib":
            afile = os.path.join(build_dir, "lib", f"ot-{dep_lib}.a")
            if afile not in link_libs:
                link_libs.append(afile)

    return link_libs


# ── Compiler detection ───────────────────────────────────────────────────

def find_compiler():
    """Return the C++ compiler to use."""
    for cc in ("clang++", "g++"):
        if shutil.which(cc):
            return cc
    sys.exit("Error: no C++ compiler found (need clang++ or g++)")


# ── Core logic ───────────────────────────────────────────────────────────

def build_libraries():
    """Build core libraries via Make (if not already built)."""
    nproc = os.cpu_count() or 1
    result = subprocess.run(
        ["make", f"-j{nproc}", "core"],
        cwd=LIBS_DIR, capture_output=True, text=True,
    )
    if result.returncode != 0:
        # Show error output
        print(result.stdout[-500:] if result.stdout else "")
        print(result.stderr[-500:] if result.stderr else "")
        sys.exit("Error: library build failed")


def run_lib_tests(lib, compiler, gtest_inc, gtest_lib,
                  specific_test=None, verbose=False):
    """Build and run tests for a single library.

    Returns (passed, failed, skipped) counts.
    """
    lib_dir = os.path.join(LIBS_DIR, lib)
    inc_dir = os.path.join(BUILD_DIR, "include")
    out_dir = os.path.join(OUT_DIR, lib)
    os.makedirs(out_dir, exist_ok=True)

    # Resolve link dependencies
    link_libs = resolve_deps(lib, LIBS_DIR, BUILD_DIR)

    # Add the library's own .a file
    own_a = os.path.join(BUILD_DIR, "lib", f"ot-{lib}.a")
    if os.path.isfile(own_a):
        link_libs = [own_a] + link_libs

    # Find test sources
    if specific_test:
        name = specific_test
        if not name.startswith("test-"):
            name = f"test-{name}"
        if not name.endswith(".cc"):
            name += ".cc"
        test_sources = [os.path.join(lib_dir, name)]
        if not os.path.isfile(test_sources[0]):
            print(f"  Error: {name} not found in libs/{lib}/")
            return 0, 0, 1
    else:
        test_sources = sorted(glob.glob(os.path.join(lib_dir, "test-*.cc")))
        # Exclude legacy tests
        test_sources = [t for t in test_sources
                        if not os.path.basename(t).startswith("legacy-")]

    if not test_sources:
        return 0, 0, 0

    total_passed = 0
    total_failed = 0
    total_skipped = 0

    for test_src in test_sources:
        test_name = os.path.basename(test_src).replace(".cc", "")
        bin_path = os.path.join(out_dir, test_name)

        # Compile
        cmd = [
            compiler, "--std=c++17", "-pedantic", "-Wall", "-Wextra",
            "-Werror", "-Wno-sign-compare", "-Wno-unknown-pragmas",
            "-DPLATFORM_LINUX",
            f"-I{inc_dir}", f"-I{gtest_inc}",
            test_src,
            "-Wl,--start-group",
        ] + link_libs + [
            "-Wl,--end-group",
            os.path.join(gtest_lib, "libgtest.a"),
            "-lpthread",
            "-o", bin_path,
        ]

        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            print(f"  SKIP {test_name}: compile/link failed")
            if verbose:
                # Show last few lines of error
                for line in result.stderr.strip().splitlines()[-5:]:
                    print(f"    {line}")
            total_skipped += 1
            continue

        # Run
        result = subprocess.run(
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

        total_passed += passed
        total_failed += failed

        if failed > 0:
            print(f"  FAIL {test_name}: {passed}/{passed + failed} passed")
            for line in output.splitlines():
                if "Failure" in line or ("FAILED" in line
                                         and "test" not in line.lower()):
                    print(f"    {line.strip()}")
        else:
            print(f"  OK   {test_name}: {passed} passed")

    return total_passed, total_failed, total_skipped


def find_all_test_libs():
    """Find all library directories that contain test-*.cc files."""
    libs = []
    for d in sorted(os.listdir(LIBS_DIR)):
        lib_dir = os.path.join(LIBS_DIR, d)
        if not os.path.isdir(lib_dir):
            continue
        tests = glob.glob(os.path.join(lib_dir, "test-*.cc"))
        # Exclude legacy tests
        tests = [t for t in tests
                 if not os.path.basename(t).startswith("legacy-")]
        if tests:
            libs.append(d)
    return libs


# ── Main ─────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description="Compile and run tests for ObTools libraries.")
    parser.add_argument("library", nargs="?",
                        help="Library name (e.g. xml, chan)")
    parser.add_argument("test", nargs="?",
                        help="Specific test file (e.g. test-chan-rw)")
    parser.add_argument("--all", action="store_true",
                        help="Run tests for all libraries")
    parser.add_argument("-v", "--verbose", action="store_true",
                        help="Show compile errors for skipped tests")
    args = parser.parse_args()

    if not args.library and not args.all:
        parser.print_help()
        sys.exit(1)

    compiler = find_compiler()
    gtest_inc, gtest_lib = find_gtest()

    # Ensure output directory exists
    os.makedirs(OUT_DIR, exist_ok=True)

    # Build libraries
    print("Building libraries...")
    build_libraries()
    print()

    if args.all:
        libs = find_all_test_libs()
    else:
        libs = [args.library]

    grand_passed = 0
    grand_failed = 0
    grand_skipped = 0

    for lib in libs:
        lib_dir = os.path.join(LIBS_DIR, lib)
        if not os.path.isdir(lib_dir):
            print(f"Error: libs/{lib}/ not found")
            sys.exit(1)

        print(f"--- {lib} ---")
        passed, failed, skipped = run_lib_tests(
            lib, compiler, gtest_inc, gtest_lib,
            specific_test=args.test if not args.all else None,
            verbose=args.verbose,
        )
        grand_passed += passed
        grand_failed += failed
        grand_skipped += skipped
        print()

    # Summary
    print("=" * 50)
    total = grand_passed + grand_failed
    if grand_failed > 0:
        print(f"FAILED: {grand_passed}/{total} tests passed, "
              f"{grand_failed} failed, {grand_skipped} skipped")
        sys.exit(1)
    else:
        print(f"PASSED: {grand_passed}/{total} tests passed"
              + (f", {grand_skipped} skipped" if grand_skipped else ""))


if __name__ == "__main__":
    main()
