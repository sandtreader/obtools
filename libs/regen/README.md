# ObTools::ReGen

Code regeneration support — merges auto-generated code with user-edited sections using marked blocks, preserving manual changes across regeneration cycles.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-regen.h"
using namespace ObTools;

// Use rofstream like ofstream — it merges with existing file on close
ReGen::rofstream out("generated.cc", "//~");
out << "// Auto-generated code\n";
out << "//~open:user-section\n";
out << "  // Default implementation\n";
out << "//~user\n";
out << "  // User can edit between user markers\n";
out << "//~end\n";
out << "//~close\n";
out.close();
```

### Marker Format

Blocks are delimited by markers (default `//~`):

```cpp
//~open:block-name     // Start of a regenerable block
  // generated code
//~user                // User-editable section starts
  // user's code preserved across regeneration
//~end                 // User-editable section ends
//~close               // End of block
```

### Direct Merge

```cpp
// Read master (generated) and user (existing) files
ifstream master_in("generated.cc.new");
ifstream user_in("generated.cc");
ofstream result("generated.cc.merged");

ReGen::MasterFile master(master_in);
ReGen::MarkedFile user(user_in);
master.merge(user, result);
```

## Build

```
NAME    = ot-regen
TYPE    = lib
```

## License

Copyright (c) 2003 Paul Clark. MIT License.
