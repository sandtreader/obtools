# ObTools::Hash

Fast 2-level integer ID hash table with O(1) lookup and coalesced chaining. Header-only.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-hash.h"
using namespace ObTools;

Hash::Table<uint32_t> table;
table.add(12345, 0);  // map ID 12345 -> index 0
auto idx = table.lookup(12345);  // returns 0
table.remove(12345);
```

## Build

```
NAME    = ot-hash
TYPE    = headers
DEPENDS = ot-mt
```

## License

Copyright (c) 2012 Paul Clark. MIT License.
