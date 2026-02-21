# ObTools::Cache

Generic evictor cache template with configurable tidy and eviction policies. Header-only.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-cache.h"
using namespace ObTools;

// LRU cache with 60-second timeout
Cache::Cache<string, MyData,
  Cache::UseTimeoutTidyPolicy<string, MyData>,
  Cache::LRUEvictorPolicy<string, MyData>> cache;

cache.add("key", data, 100);  // limit 100 entries
MyData *p = cache.lookup("key");
cache.tidy();  // evict expired
```

## Build

```
NAME    = ot-cache
TYPE    = headers
DEPENDS = ot-mt
```

## License

Copyright (c) 2009 Paul Clark. MIT License.
