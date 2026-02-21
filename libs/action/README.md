# ObTools::Action

Generic asynchronous action management with typed action queues and multi-handler dispatch. Header-only.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-action.h"
using namespace ObTools;

// Define action types
enum ActionType { ping, pong };

// Create a concrete action
class PingAction: public Action::Action<ActionType>
{
  int value;
public:
  PingAction(int v): value(v) {}
  ActionType get_type() const override { return ping; }
  int get_value() const { return value; }
};

// Create a handler
class PingHandler: public Action::Manager<ActionType>::Handler
{
public:
  void handle(const Action::Action<ActionType>& action) override
  {
    auto& p = dynamic_cast<const PingAction&>(action);
    cout << "Ping: " << p.get_value() << endl;
  }
};

int main()
{
  Action::Manager<ActionType> manager;
  PingHandler handler;
  manager.add_handler(ping, handler);
  manager.set_queue_limit(100);

  manager.queue(new PingAction(42));
  // handler processes action asynchronously
}
```

## Build

```
NAME    = ot-action
TYPE    = headers
DEPENDS = ot-gen ot-mt
```

## License

Copyright (c) 2012 Paul Clark. MIT License.
