# ObTools::Script

XML-based scripting language for composable test/workflow actions with parallel execution, timing, and variable scoping.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-script.h"
using namespace ObTools;

Script::BaseLanguage lang;
// Register custom actions...
Script::Script script(lang);
script.read_from(xml_config);
script.run();
```

## Build

```
NAME    = ot-script
TYPE    = lib
DEPENDS = ot-xml ot-init ot-misc ot-time ot-log
```

## License

Copyright (c) 2012 Paul Clark. MIT License.
