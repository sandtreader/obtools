# CLAUDE.md - ObTools::File Library

## Overview

`ObTools::File` provides cross-platform file and directory manipulation: path parsing, file metadata, reading/writing entire files, directory iteration with glob patterns, and platform-aware streams. Lives under `namespace ObTools::File`.

**Header:** `ot-file.h`
**Dependencies:** `ot-gen`, `ot-text`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Path` | Path manipulation, file I/O, metadata, permissions |
| `Directory` | Directory operations, glob-based inspection |
| `InStream` / `OutStream` | Platform-aware file streams |
| `BufferedOutStream` | Buffered file output with configurable size |
| `MultiOutStream` | Write to multiple files simultaneously |

## Path

```cpp
Path(const string& path);  Path(const Directory& dir, const string& leaf);

// Components
string str() const;
string dirname() const;       string leafname() const;
string extension() const;     string basename() const;  // leaf without extension
bool is_absolute() const;
Path realpath() const;         Path resolve(const Path& base) const;

// Metadata
bool exists() const;  bool is_dir() const;  bool is_link() const;
uint64_t length() const;
Time::Stamp last_modified() const;
bool readable() const;  bool writeable() const;

// Operations
bool set_mode(int mode) const;   bool set_ownership(int uid, int gid) const;
bool erase() const;  bool touch() const;  bool rename(const Path& new_path) const;

// File I/O
string read_all() const;                    // read entire file to string
bool write_all(const string& data) const;   // write string to file (atomic via temp)

// User/group helpers
static int user_to_uid(const string& user);
static int group_to_gid(const string& group);
```

## Directory

```cpp
Directory(const string& path);

bool ensure(int mode=0777) const;        // create if missing (recursive)
bool is_dir() const;  bool empty() const;
list<Path> inspect(const string& pattern="*") const;
list<Path> inspect_recursive(const string& pattern="*") const;
```

## File Layout

```
ot-file.h            - Public header
path.cc              - Path implementation
directory.cc         - Directory implementation
stream.cc            - InStream/OutStream
buffered-stream.cc   - BufferedOutStream
multi-stream.cc      - MultiOutStream
test-path.cc         - Path tests (gtest)
```
