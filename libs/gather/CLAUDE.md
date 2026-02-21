# CLAUDE.md - ObTools::Gather Library

## Overview

`ObTools::Gather` provides a zero-copy multi-segment gather buffer for efficient network packet assembly. Lives under `namespace ObTools::Gather`.

**Header:** `ot-gather.h`
**Dependencies:** `ot-chan`, `ot-misc`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Segment` | Individual buffer segment (owned or external) |
| `Buffer` | Gather buffer managing segment array |
| `BufferIterator` | Bidirectional iterator over segments |
| `Reader` | Channel::Reader adapter for Buffer |

## Buffer

```cpp
Buffer(int max_segments);

void add(const unsigned char *data, size_t length);   // external ref
unsigned char *add(size_t length);                     // allocate owned
void add(const Buffer& other);                         // append buffer

void insert(int pos, ...);   // insert at position (same overloads)
void consume(size_t n);      // remove from front
void limit(size_t n);        // truncate to n bytes
void reset();                // clear all
void tidy();                 // compact segments

size_t get_length() const;
void copy(unsigned char *dest) const;
unsigned char *get_flat_data();   // flatten to contiguous
void fill(struct iovec *iov, int& n) const;   // for sendmsg

iterator begin(); iterator end();
```
