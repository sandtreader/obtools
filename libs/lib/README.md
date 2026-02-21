# ObTools::Lib

Cross-platform dynamic shared library (`.so`/`.dll`) loading with function lookup. Header-only.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-lib.h"
using namespace ObTools;

Lib::Library lib("./libplugin.so");
if (!lib)
{
  cerr << "Failed to load: " << lib.get_error() << endl;
  return 1;
}

// Look up a function
using InitFunc = int (*)(const char*);
auto init = lib.get_function<InitFunc>("plugin_init");
if (init)
  init("config");
```

## Build

```
NAME           = ot-library
TYPE           = headers
LINUX-DEPENDS  = ext-dl
```

## License

Copyright (c) 2017 Paul Clark. MIT License.
