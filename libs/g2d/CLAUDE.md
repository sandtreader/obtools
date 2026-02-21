# CLAUDE.md - ObTools::G2D Library

## Overview

`ObTools::G2D` provides basic 2D graphics types. Header-only. Lives under `namespace ObTools::G2D`.

**Header:** `ot-g2d.h`
**Dependencies:** none
**Type:** headers

## Key Types

| Type | Purpose |
|------|---------|
| `coord_t` | Coordinate type (`double`) |
| `Point` | 2D Cartesian point |

## Point

```cpp
typedef double coord_t;

struct Point {
  coord_t x;
  coord_t y;
  Point();                        // (0.0, 0.0)
  Point(coord_t _x, coord_t _y);
};
```
