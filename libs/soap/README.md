# ObTools::SOAP

SOAP message creation, parsing, and HTTP transport for web services.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-soap.h"
using namespace ObTools;

// Build and send SOAP message
SOAP::Message msg;
XML::Element *body = new XML::Element("GetPrice");
body->add("item", "Widget");
msg.add_body(body);

SOAP::HTTPClient client(Net::EndPoint("api.example.com", 80));
SOAP::Message response;
client.post(Web::URL("/soap"), msg, response);
```

## Build

```
NAME    = ot-soap
TYPE    = lib
DEPENDS = ot-web ot-msg ot-init
```

## License

Copyright (c) 2003 Paul Clark. MIT License.
