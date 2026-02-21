# CLAUDE.md - ObTools::Hash Library

## Overview

`ObTools::Hash` is a fast 2-level integer ID hash table with O(1) lookup and coalesced chaining for collision handling. Header-only. Lives under `namespace ObTools::Hash`.

**Header:** `ot-hash.h`
**Dependencies:** `ot-mt`
**Type:** headers

## Key Classes

| Class | Purpose |
|-------|---------|
| `Table<ID_T,...>` | Two-level hash table for integer IDs |
| `Block<...>` | Second-level hash block with internal freelist |
| `Entry<...>` | Individual entry with chaining pointers |
| `Stats` | Hash statistics (entries, fullness, chain length) |

## Table Methods

```cpp
bool add(ID_T id, INDEX_T index);
INDEX_T lookup(ID_T id);                    // INVALID_INDEX if not found
INDEX_T lookup_and_remove(ID_T id);
void remove(ID_T id);
bool check(ID_T id);
Stats get_stats();
void dump(ostream& s);
```
