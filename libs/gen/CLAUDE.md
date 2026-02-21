# CLAUDE.md - ObTools::Gen Library

## Overview

`ObTools::Gen` provides generic utility types: type-safe identifiers, compile-time maps, three-valued logic, and bidirectional shifts. Header-only. Lives under `namespace ObTools`.

**Header:** `ot-gen.h`
**Dependencies:** none
**Type:** headers

## Key Types

| Type | Purpose |
|------|---------|
| `Tristate` | Three-valued enum: `unset`, `on`, `off` |
| `Id<T>` | Type-safe string identifier (tag type T) |
| `ConstExprMap<K,V,S>` | Compile-time constexpr key-value map |

## Tristate

```cpp
enum class Tristate { unset, on, off };
```

## Id<T>

```cpp
template<typename T> class Id : public string {
  Id();
  explicit Id(const string& id);
  explicit operator bool() const;  // true if non-empty
  // String mutation operators are hidden
};
```

## ConstExprMap<K,V,S>

```cpp
template<typename K, typename V, size_t S>
struct ConstExprMap {
  array<pair<K,V>, S> data;
  constexpr V lookup(const K& key) const;          // throws range_error
  constexpr K reverse_lookup(const V& value) const; // throws range_error
};
```

## Utility Functions

```cpp
template<typename T> T shiftr(const T& value, int places);  // bidirectional right shift
template<typename T> T shiftl(const T& value, int places);  // bidirectional left shift
```
