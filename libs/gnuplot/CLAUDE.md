# CLAUDE.md - ObTools::Gnuplot Library

## Overview

`ObTools::Gnuplot` generates gnuplot command syntax for plotting 2D data as PNG. Header-only. Lives under `namespace ObTools::Gnuplot`.

**Header:** `ot-gnuplot.h`
**Dependencies:** none
**Type:** headers

## Key Classes

| Class | Purpose |
|-------|---------|
| `Output` | Generates gnuplot inline data plot commands |

## Output

```cpp
Output(ostream& os, const string& label);   // writes header
void add_point(double x, double y);          // writes data point
~Output();                                    // writes EOF marker
```

## Generated Format

```
set terminal png
plot '-' using 1:2 title "label" with lines
x1    y1
x2    y2
...
EOF
```
