# ObTools::Log

Hierarchical logging framework with multiple channels, filters, and severity levels.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-log.h"
using namespace ObTools;

// Setup: log errors to stderr, everything to file
Log::StreamChannel stderr_chan(cerr);
Log::OwnedStreamChannel file_chan(new ofstream("app.log"));
Log::LevelFilter error_filter(Log::Level::error, stderr_chan);
Log::TimestampFilter ts_filter(file_chan);

Log::logger.connect(&error_filter);
Log::logger.connect_full(&ts_filter, Log::Level::dump);

// Usage
Log::Error log; log << "Something failed" << endl;
Log::Summary log2; log2 << "Server started on port 8080" << endl;
Log::Detail log3; log3 << "Processing request" << endl;
```

## Build

```
NAME    = ot-log
TYPE    = lib
DEPENDS = ot-mt ot-time
```

## License

Copyright (c) 2003 Paul Clark. MIT License.
