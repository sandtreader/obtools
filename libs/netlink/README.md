# ObTools::Netlink

C++ wrapper for Linux Generic Netlink protocol, enabling type-safe message construction and attribute parsing.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-netlink.h"
using namespace ObTools;

// Connect to a generic netlink family
Netlink::GenericNetlink nl("nlctrl");
if (!nl.valid()) return 1;

// Define a request
class MyRequest: public Netlink::GenericRequest
{
public:
  MyRequest(const Netlink::GenericNetlink& nl)
    : GenericRequest(nl, CTRL_CMD_GETFAMILY, 1) {}

  int get_attribute_count() override { return CTRL_ATTR_MAX; }

  int callback(Netlink::GenericResponse& response) override
  {
    string name = response.get_string(CTRL_ATTR_FAMILY_NAME);
    uint16_t id = response.get_uint16(CTRL_ATTR_FAMILY_ID);
    cout << "Family: " << name << " ID: " << id << endl;
    return 0;
  }
};

MyRequest request(nl);
request.set_string(CTRL_ATTR_FAMILY_NAME, "nlctrl");
nl.send(request);
```

### Nested Attributes

```cpp
int callback(Netlink::GenericResponse& response) override
{
  vector<struct nlattr *> nested;
  if (response.get_nested_attrs(CTRL_ATTR_MCAST_GROUPS, nested,
                                 CTRL_ATTR_MCAST_GRP_MAX, policy))
  {
    string grp_name = response.get_string(CTRL_ATTR_MCAST_GRP_NAME, nested);
  }
  return 0;
}
```

## Build

```
NAME    = ot-netlink
TYPE    = lib
DEPENDS = ext-pkg-libnl-3.0 ext-pkg-libnl-genl-3.0
```

## License

Copyright (c) 2017 Paul Clark. MIT License.
