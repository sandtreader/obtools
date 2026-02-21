# CLAUDE.md - ObTools::Lib Library

## Overview

`ObTools::Lib` provides cross-platform dynamic shared library (`.so`/`.dll`) loading with function lookup. Header-only. Lives under `namespace ObTools::Lib`.

**Header:** `ot-lib.h`
**Dependencies:** `ext-dl` (Linux)
**Type:** headers

## Key Classes

| Class | Purpose |
|-------|---------|
| `Library` | Dynamic library loader with function lookup |

## Library

```cpp
Library(const string& path);                      // dlopen / LoadLibrary
explicit operator bool() const;                    // true if loaded
template<typename T> T get_function(const string& name) const;  // dlsym / GetProcAddress
string get_error() const;                          // dlerror / "unknown"
~Library();                                        // dlclose / FreeLibrary
```

## Platform Support

- **Linux:** `dlopen(path, RTLD_NOW)`, `dlsym()`, `dlclose()`
- **Windows:** `LoadLibrary()`, `GetProcAddress()`, `FreeLibrary()`
