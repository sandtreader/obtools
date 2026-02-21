# ObTools::Gnuplot

Generates gnuplot command syntax for plotting 2D data as PNG. Header-only.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-gnuplot.h"
using namespace ObTools;

// Write gnuplot commands to file
ofstream f("plot.gp");
{
  Gnuplot::Output plot(f, "Sine Wave");
  for (double x = 0; x < 6.28; x += 0.1)
    plot.add_point(x, sin(x));
}  // destructor writes EOF

// Then: gnuplot plot.gp > plot.png
```

### Pipe to gnuplot

```cpp
// Or pipe directly
FILE *gp = popen("gnuplot > output.png", "w");
// Use with ostream wrapper...
```

## Build

```
NAME    = ot-gnuplot
TYPE    = headers
```

## License

Copyright (c) 2018 Paul Clark. MIT License.
