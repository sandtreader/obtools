# CLAUDE.md - ObTools::Cache Library

## Overview

`ObTools::Cache` is a generic evictor cache template with configurable tidy and eviction policies. Header-only. Lives under `namespace ObTools::Cache`.

**Header:** `ot-cache.h`
**Dependencies:** `ot-mt`
**Type:** headers

## Key Classes

| Class | Purpose |
|-------|---------|
| `Cache<ID,CONTENT,TIDY,EVICTOR>` | Main cache template (map-based) |
| `PointerCache<ID,CONTENT,TIDY,EVICTOR>` | Cache owning dynamically-allocated objects |
| `TidyPolicy<ID,CONTENT>` | Abstract: background eviction |
| `EvictorPolicy<ID,CONTENT>` | Abstract: emergency eviction |

## Built-in Policies

| Policy | Description |
|--------|-------------|
| `NoTidyPolicy` | No background eviction |
| `UseTimeoutTidyPolicy` | Evict after timeout since last use |
| `AgeTimeoutTidyPolicy` | Evict after timeout since creation |
| `NoEvictorPolicy` | No emergency eviction |
| `LRUEvictorPolicy` | Evict least recently used |
| `AgeEvictorPolicy` | Evict oldest entry |

## Cache Methods

```cpp
bool add(const ID& id, const CONTENT& content, int limit=0);
CONTENT *lookup(const ID& id);
bool contains(const ID& id);
void remove(const ID& id);
void touch(const ID& id);
void tidy();    // run tidy policy
void evict();   // run evictor policy
void clear();
int count() const;
```
