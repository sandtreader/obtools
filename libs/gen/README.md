# ObTools::Gen

Generic utility types: type-safe identifiers, compile-time maps, three-valued logic, and bidirectional shifts. Header-only.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

### Type-Safe Identifiers

```cpp
#include "ot-gen.h"

struct UserTag {};
struct OrderTag {};
using UserId = ObTools::Id<UserTag>;
using OrderId = ObTools::Id<OrderTag>;

UserId user(string("u-123"));
OrderId order(string("o-456"));
// user = order;  // compile error — different tag types
```

### Compile-Time Map

```cpp
static constexpr ObTools::ConstExprMap<int, const char*, 3> colors = {{
  {0, "red"}, {1, "green"}, {2, "blue"}
}};

constexpr auto name = colors.lookup(1);           // "green"
constexpr auto id = colors.reverse_lookup("blue"); // 2
```

### Tristate

```cpp
ObTools::Tristate flag = ObTools::Tristate::unset;
if (flag == ObTools::Tristate::on) { /* ... */ }
```

## Build

```
NAME    = ot-gen
TYPE    = headers
```

## License

Copyright (c) 2018 Paul Clark. MIT License.
