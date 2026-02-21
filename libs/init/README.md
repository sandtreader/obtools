# ObTools::Init

Automatic initialization sequencing and factory/registry pattern for plugin-style object creation.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

### Initialization Sequencing

```cpp
#include "ot-init.h"
using namespace ObTools;

class MyInit: public Init::AutoAction
{
public:
  MyInit(): Init::AutoAction(0) {}  // rank 0 = runs first
  void initialise() override
  {
    cout << "Initialised!" << endl;
  }
};

// Static instance auto-registers
static MyInit my_init;

int main()
{
  Init::Sequence::run();  // runs all registered actions in rank order
}
```

### Factory Registry

```cpp
// Abstract base
class Transport { public: virtual ~Transport() {} };

// Concrete implementations
class TCPTransport: public Transport { /* ... */ };
class UDPTransport: public Transport { /* ... */ };

// Registry
Init::Registry<Transport> transport_registry;

// Auto-register factories (typically as static objects)
Init::AutoRegister<Transport, TCPTransport> tcp_reg(transport_registry, "tcp");
Init::AutoRegister<Transport, UDPTransport> udp_reg(transport_registry, "udp");

// After Init::Sequence::run():
Transport *t = transport_registry.create("tcp", config_element);
```

## Build

```
NAME    = ot-init
TYPE    = lib
DEPENDS = ot-xml
```

## License

Copyright (c) 2003 Paul Clark. MIT License.
