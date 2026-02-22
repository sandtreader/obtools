# CLAUDE.md — ObTools Codebase Guide

## Project Overview

ObTools is a collection of C++17 utility libraries for high-performance Unix servers and applications, created by Paul Clark in 2003. It is licensed under MIT. The libraries provide XML/JSON parsing, multithreading, crypto (OpenSSL), database interfaces (MySQL/PostgreSQL/SQLite), networking, HTTP/SOAP clients and servers, daemon framework, logging, and more.

### Major Components

| Directory | Description |
|-----------|-------------|
| `libs/` | 50+ core utility libraries (the heart of the project) |
| `xmlmesh/` | XML-based publish-subscribe messaging system (server, client, listener, bindings for C++/C/PHP/Perl/JS) |
| `obcache/` | Automated ORM code generation experiment (gen, libs, tests) |
| `tools/` | Code generation tools (toolgen, xmitoolgen, xmltoolgen, rest-test) |
| `build/` | Build system configuration, scripts, and rules |
| `templates/` | C++ templates for new source/header files |

## Build System

The project uses **Tup** as its primary build system with Lua-based rules. A legacy **Makefile** fallback exists for library-only builds.

### Prerequisites (Ubuntu/Debian)

```bash
sudo apt install build-essential tup clang git pkg-config debhelper dh-exec
sudo apt install libssl-dev libsqlite3-dev libmysqlclient-dev libpq-dev libnl-genl-3-dev
# For MariaDB: libmariadb-dev-compat instead of libmysqlclient-dev
```

### Installing Google Test

```bash
sudo apt install libgtest-dev cmake
cd /usr/src/gtest
sudo cmake CMakeLists.txt
sudo make
sudo cp lib/*.a /usr/lib/
```

### Build Commands

```bash
# Initialize debug build (with tests)
build/init.sh -t debug
tup

# Initialize release build
build/init.sh -t release
tup

# Simple library-only build via Make
cd libs && make
```

Build types: `debug`, `release`, `test`. Platforms: `linux` (default), `windows` (MinGW cross-compile), `web` (Emscripten/WASM).

### Build Configuration

- Config files in `build/`: `tup.config.debug`, `tup.config.release`, `tup.config.test`, etc.
- `build/init.sh -t <type> [-p <platform>]` creates a `build-<type>` directory with the correct config
- Build outputs go to `build-debug/` or `build-release/` directories

### Tupfile Format

Each library/component has a `Tupfile`:

```
NAME    = ot-<library>
TYPE    = lib          # lib | shared | exe | headers | package
DEPENDS = ot-dep1 ot-dep2
# Optional:
WINDOWS-DEPENDS = ext-wsock32
LINUX-DEPENDS = ext-pkg-libnl-genl-3.0
PLATFORMS = posix      # Restrict to specific platforms

include_rules
```

Dependencies are resolved transitively by `build/Tuprules.lua` via `Tuppath.lua`.

### Packaging

- **Debian**: `build/create-deb.sh` — each packaged component has a `DEBIAN/` directory with `control`, `install`, etc.
- **RPM**: `build/create-rpm.sh` — converts from Debian packaging
- **Windows**: `build/create-nsis.sh` — NSIS installer with `WINDOWS/` config directories
- **WASM**: `build/create-wasm.sh` — Emscripten packaging with `WEB/` config directories

## Testing

- **Framework**: Google Test (gtest)
- **Test files**: Named `test-<feature>.cc` or `test-<feature>-gtest.cc` in each library directory
- **Running tests**: Tests run automatically during debug builds when `CONFIG_TEST=y`
- Tests are compiled and executed as part of the Tup build pipeline — a failing test fails the build

### Test File Structure

```cpp
#include <gtest/gtest.h>
#include "ot-<library>.h"

namespace {
using namespace std;
using namespace ObTools;

TEST(FeatureTest, TestDescription)
{
  // Arrange, Act, Assert
  EXPECT_EQ(expected, actual);
  ASSERT_TRUE(condition);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
```

### Code Coverage

Line-level code coverage can be measured for any library using `build/run-coverage.py`. The script uses `g++` with `--coverage` and `gcov` — no external tools beyond the standard toolchain.

```bash
# Basic coverage report
build/run-coverage.py xml

# Show uncovered lines
build/run-coverage.py xml -v

# Show per-function coverage
build/run-coverage.py xml -f

# Combine both
build/run-coverage.py json -v -f
```

The script automatically:
- Builds the core libraries via Make (if not already built)
- Resolves transitive link dependencies by parsing each library's `Tupfile` `DEPENDS` line recursively
- Compiles the library's `.cc` source files with `--coverage` instrumentation
- Builds and runs every `test-*.cc` file found in the library directory
- Reports per-file line coverage and totals
- Auto-detects or builds Google Test from source if not installed

**Prerequisites**: `g++`, `gcov` (included with `build-essential`). Google Test is auto-provisioned if missing.

**Output directory**: Coverage artefacts are written to `/tmp/obtools-coverage/` and cleaned up from the working tree automatically.

**Interpreting results**: The `const` overloads of methods (e.g., `get_child() const` vs `get_child()`) share identical logic, so tests typically exercise only one variant. This means ~90% line coverage on a well-tested library is normal — the remaining ~10% is usually `const` duplicates, error-recovery paths, and rarely-reached branches.

## Code Conventions

### File Naming

- Headers: `ot-<library>.h` (e.g., `ot-json.h`, `ot-net.h`, `ot-llm.h`)
- Implementation: descriptive `<functionality>.cc` (e.g., `address.cc`, `socket.cc`, `parser.cc`)
- Tests: `test-<feature>.cc` or `test-<feature>-gtest.cc`
- Build config: `Tupfile` in each component directory

### Header File Format

```cpp
//==========================================================================
// ObTools::<Library>: ot-<library>.h
//
// Public definitions for ObTools::<Library>
//
// Copyright (c) <year> Paul Clark.
//==========================================================================

#ifndef __OBTOOLS_LIBRARY_H
#define __OBTOOLS_LIBRARY_H

#include <standard-lib>
#include "ot-dependency.h"

namespace ObTools { namespace Library {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Class description
class ClassName
{
  // private members
public:
  // public interface
};

}} //namespaces

#endif // !__OBTOOLS_LIBRARY_H
```

### Implementation File Format

```cpp
//==========================================================================
// ObTools::<Library>: <file>.cc
//
// <Description>
//
// Copyright (c) <year> Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-<library>.h"

namespace ObTools { namespace Library {

// Implementation

}} // namespaces
```

### Naming Conventions

| Element | Style | Examples |
|---------|-------|---------|
| Classes/Structs | PascalCase | `TCPSocket`, `IPAddress`, `Value`, `Interface` |
| Methods/Functions | snake_case | `get_hostname()`, `read_exact()`, `is_broadcast()` |
| Variables | snake_case | `host_byte_order`, `in_buf_size` |
| Constants/Enums | UPPER_CASE | `INVALID_FD`, `DEFAULT_IN_BUFFER` |
| Namespaces | PascalCase | `ObTools::Net`, `ObTools::JSON`, `ObTools::LLM` |
| File names | lowercase with hyphens | `ot-json.h`, `test-url-gtest.cc` |

### Formatting

- **Indentation**: 2 spaces (no tabs)
- **Brace style**: Opening brace on same line (`class Foo {`, `if (x) {`)
- **Namespace closing**: `}} //namespaces`
- **Include guards**: `#ifndef __OBTOOLS_LIBRARY_H` (never `#pragma once`)
- **Section separators**: `//====...` for major sections, `//----...` for subsections
- **No `.clang-format` or `.editorconfig`** — conventions are maintained manually

### Namespace Pattern

All code lives under a two-level namespace:

```cpp
namespace ObTools { namespace Library {
  using namespace std;
  // ...
}} //namespaces
```

### Error Handling

- Custom exception structs with descriptive `error` string member
- Boolean return values for success/failure on I/O operations
- `operator bool()` / `operator!()` for validity checks on objects

### Platform-Specific Code

```cpp
#if defined(PLATFORM_WINDOWS)
  // Windows-specific
#elif defined(PLATFORM_MACOS)
  // macOS-specific
#else
  // Linux (default)
#endif
```

Platform defines: `PLATFORM_LINUX`, `PLATFORM_WINDOWS`, `PLATFORM_WEB`

## Compiler Settings

- **Compiler**: `clang++` (Linux), `x86_64-w64-mingw32-g++` (Windows), `em++` (Web)
- **Standard**: C++17 (`--std=c++17`)
- **Warning flags**: `-pedantic -Wall -Wextra -Werror` — all warnings are errors
- **Debug flags**: `-ggdb3 -gdwarf-4 -DDEBUG -D_GLIBCXX_DEBUG`
- **Release flags**: `-O2`

## Library Architecture

Libraries follow a layered dependency architecture:

```
                    ┌─────────────────────────┐
                    │   Applications/Services  │
                    │  (xmlmesh, obcache, etc) │
                    └──────────┬──────────────┘
                               │
              ┌────────────────┼─────────────────┐
              │                │                  │
        ┌─────┴─────┐  ┌──────┴──────┐  ┌───────┴───────┐
        │ Networking │  │  Database   │  │   Services    │
        │ net, ssl,  │  │ db, db-mysql│  │ web, soap,    │
        │ web, dns   │  │ db-pgsql,   │  │ llm, xmlmesh  │
        └─────┬──────┘  │ db-sqlite   │  └───────┬───────┘
              │         └──────┬──────┘          │
              │                │                  │
        ┌─────┴────────────────┴──────────────────┴──┐
        │              Core Utilities                  │
        │  text, json, xml, lex, chan, mt, log, time,  │
        │  crypto, file, misc, gen, hash, exec, msg    │
        └──────────────────────────────────────────────┘
```

### Key Libraries

| Library | Header | Purpose |
|---------|--------|---------|
| `xml` | `ot-xml.h` | XML parser and DOM |
| `json` | `ot-json.h` | JSON parser/writer with CBOR support |
| `net` | `ot-net.h` | TCP/UDP networking, sockets |
| `ssl` / `ssl-openssl` | `ot-ssl.h` | TLS/SSL support over OpenSSL |
| `web` | `ot-web.h` | HTTP client and server |
| `mt` | `ot-mt.h` | Multithreading primitives |
| `db` | `ot-db.h` | Database interface (abstract) |
| `crypto` | `ot-crypto.h` | Cryptographic operations (OpenSSL) |
| `log` | `ot-log.h` | Logging framework |
| `text` | `ot-text.h` | String manipulation utilities |
| `time` | `ot-time.h` | Timestamp handling |
| `daemon` | `ot-daemon.h` | Linux daemon framework |
| `llm` | `ot-llm.h` | LLM interface (OpenAI, etc.) |
| `chan` | `ot-chan.h` | Binary channel protocol |
| `lex` | `ot-lex.h` | Lexical analysis |

## Adding a New Library

1. Create directory: `libs/<name>/`
2. Create header: `libs/<name>/ot-<name>.h` following the header template
3. Create implementation files: `libs/<name>/<feature>.cc`
4. Create test files: `libs/<name>/test-<feature>.cc`
5. Create `libs/<name>/Tupfile`:
   ```
   NAME    = ot-<name>
   TYPE    = lib
   DEPENDS = ot-dep1 ot-dep2
   include_rules
   ```
6. Add to `libs/Tupfile` package dependencies if needed
7. If the library should be accessible from other projects, add mapping in `Tuppath.lua`

## Common Patterns for AI Assistants

- When modifying a library, always read the corresponding `ot-<library>.h` header first to understand the public API
- Look at the `Tupfile` to understand a component's dependencies
- Run `tup` in a build directory to compile and test; a failing test will fail the build
- The Makefile in `libs/` is a simpler alternative for building just the libraries: `cd libs && make`
- All source files start with the standard copyright banner — preserve it
- Keep methods in snake_case even if surrounding code uses different conventions elsewhere
- Use 2-space indentation consistently
- When adding tests, follow the gtest patterns in existing `test-*.cc` files
- Dependencies between libraries must be declared in the `Tupfile` `DEPENDS` line
- External system libraries use `ext-` prefix (e.g., `ext-pthread`) or `ext-pkg-` for pkg-config
