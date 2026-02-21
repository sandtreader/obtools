# ObTools::XMI

XMI (XML Metadata Interchange) parser for UML model deserialisation from XML.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-xmi.h"
using namespace ObTools;

XMI::Reader reader(cerr);
ifstream f("model.xmi");
reader.read_from(f);

if (reader.model)
  // traverse reader.model
```

## Build

```
NAME    = ot-xmi
TYPE    = lib
DEPENDS = ot-xml ot-uml
```

## License

Copyright (c) 2003 Paul Clark. MIT License.
