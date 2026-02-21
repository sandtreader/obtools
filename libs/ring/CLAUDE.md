# CLAUDE.md - ObTools::Ring Library

## Overview

`ObTools::Ring` provides a lock-free, fixed-size ring buffer for single-writer, single-reader communication. Header-only. Lives under `namespace ObTools::Ring`.

**Header:** `ot-ring.h`
**Dependencies:** none
**Type:** headers

## Key Classes

| Class | Purpose |
|-------|---------|
| `Buffer<ITEM_T>` | Lock-free SPSC ring buffer |

## Buffer<ITEM_T>

```cpp
Buffer(int length);                    // allocates length+1 items

bool put(const ITEM_T& item);         // false if full
bool get(ITEM_T& item_p);             // false if empty

void flush_from_put();                 // clear (writer side)
void flush_from_get();                 // clear (reader side)

unsigned size() const;                 // allocated capacity
unsigned used() const;                 // current item count
```

## Design

- Uses `std::atomic<unsigned>` for lock-free synchronization
- Single-writer, single-reader only (no locks needed)
- Circular buffer with modular arithmetic
- Empty when `in_index == out_index`
- Full when `in_index == out_index - 1 (mod size)`
- No dynamic allocation after construction
