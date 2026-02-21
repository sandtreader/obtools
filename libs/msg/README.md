# ObTools::Message

Generic message/RPC broker supporting multiple transports and XML-configured handlers. Header-only.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-msg.h"
using namespace ObTools;

struct MyContext { /* application context */ };

// Define a handler
class EchoHandler: public Message::Handler<MyContext>
{
public:
  EchoHandler(const XML::Element& cfg)
    : Message::Handler<MyContext>(cfg, "echo") {}

  void handle_message(MyContext& ctx, const XML::Element& request,
                     const SSL::ClientDetails& client,
                     XML::Element& response) override
  {
    response = request;  // echo back
  }
};

// Define a transport
class MyTransport: public Message::Transport<MyContext>
{
public:
  MyTransport(): Message::Transport<MyContext>("my-transport") {}

  void register_handler(Message::Handler<MyContext>& handler,
                        const XML::Element& config) override
  {
    // Register handler for this transport
  }
};

// Wire it together
Init::Registry<Message::Handler<MyContext>> handler_registry;
Message::Broker<MyContext> broker(handler_registry);
broker.add_transport(new MyTransport());
broker.configure(config_element);
```

## Build

```
NAME    = ot-msg
TYPE    = headers
DEPENDS = ot-xml ot-init ot-ssl
```

## License

Copyright (c) 2011 Paul Clark. MIT License.
