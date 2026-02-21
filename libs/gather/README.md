# ObTools::Gather

Zero-copy multi-segment gather buffer for efficient network packet assembly. Manages arrays of buffer segments that can be sent with `sendmsg()` without copying.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-gather.h"
using namespace ObTools;

Gather::Buffer buf(16);  // max 16 segments
buf.add(header_data, header_len);     // reference external
unsigned char *p = buf.add(100);      // allocate 100 bytes
memcpy(p, payload, 100);

size_t total = buf.get_length();
unsigned char *flat = buf.get_flat_data();  // contiguous copy
```

## Build

```
NAME    = ot-gather
TYPE    = lib
DEPENDS = ot-chan ot-misc
```

## License

Copyright (c) 2010 Paul Clark. MIT License.
