# ObTools::Exec

Helpers for spawning and executing sub-processes with stdin/stdout/stderr handling.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-exec.h"
using namespace ObTools;

// Run a command and capture output
Exec::Command cmd("/bin/echo Hello World");
string output;
if (cmd.execute(output))
  cout << "Output: " << output << endl;

// Run with stdin input
Exec::Command cat("/bin/cat");
string result;
cat.execute("Hello from stdin", result);

// Fire and forget
Exec::Command date("/bin/date");
date.execute();
```

## Build

```
NAME    = ot-exec
TYPE    = lib
DEPENDS = ot-log ot-text
```

## License

Copyright (c) 2008 Paul Clark. MIT License.
