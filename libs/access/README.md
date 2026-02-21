# ObTools::Access

XML-configured access control (ACL) system with groups, users, IP addresses, and resource matching. Supports glob patterns for user and resource names.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-access.h"
using namespace ObTools;

// XML configuration
string xml = R"(
<access>
  <group id="admins">
    <user>alice</user>
    <user>bob</user>
  </group>
  <resource name="/admin/*">
    <allow group="admins"/>
  </resource>
  <resource name="/public/*">
    <allow address="0.0.0.0/0"/>
  </resource>
</access>
)";

XML::Parser parser;
auto doc = parser.read_from(xml);
Access::Checker checker(doc.get_root());

Net::IPAddress addr("192.168.1.1");

// Check access
bool allowed = checker.check("/admin/settings", addr, "alice");  // true
allowed = checker.check("/admin/settings", addr, "mallory");     // false
allowed = checker.check("/public/index.html", addr, "anyone");   // true
```

### SSL Client Certificate

```cpp
// Check using SSL client certificate CN
SSL::ClientDetails client;
client.cn = "alice";
bool allowed = checker.check("/admin/settings", client);
```

## Build

```
NAME    = ot-access
TYPE    = lib
DEPENDS = ot-xml ot-ssl
```

## License

Copyright (c) 2003 Paul Clark. MIT License.
