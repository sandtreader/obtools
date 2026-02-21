# ObTools::File

Cross-platform file and directory manipulation for C++17. Provides path parsing, file metadata, atomic read/write, directory iteration with glob patterns, and platform-aware streams.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-file.h"
using namespace ObTools;

// Path manipulation
File::Path p("/home/user/docs/file.txt");
p.dirname();    // "/home/user/docs"
p.leafname();   // "file.txt"
p.extension();  // "txt"
p.basename();   // "file"

// File I/O
string content = p.read_all();
p.write_all("new content");

// Metadata
if (p.exists() && p.readable())
  cout << p.length() << " bytes, modified " << p.last_modified().iso() << endl;

// Directory
File::Directory dir("/tmp/myapp");
dir.ensure();  // create recursively if needed
for (auto& f : dir.inspect("*.txt"))
  cout << f.str() << endl;
for (auto& f : dir.inspect_recursive("*.log"))
  cout << f.str() << endl;
```

## Build

```
NAME    = ot-file
TYPE    = lib
DEPENDS = ot-gen ot-text
```

## License

Copyright (c) 2003 Paul Clark. MIT License.
