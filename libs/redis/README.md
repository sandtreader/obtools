# ObTools::Redis

Minimal Redis client - RESP2 protocol over TCP, with one kept connection, pipelining and convenience methods for GET, SET, SCAN and HGETALL.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-redis.h"
using namespace ObTools;

Redis::Options options;
options.server = Net::EndPoint(Net::IPAddress("127.0.0.1"), 6379);
options.password = "secret";      // AUTH, if set
options.database = 2;             // SELECT, if not 0

Redis::Client redis(options);     // Connects on first use

try
{
  redis.set("greeting", "hello");
  auto v = redis.get("greeting");
  if (v) cout << *v << endl;

  // Any command - error replies come back as values
  auto reply = redis.command({"INCR", "counter"});
  if (!reply.is_error()) cout << reply.integer << endl;

  // Scan for keys, then fetch their hashes, pipelined
  auto keys = redis.scan("device:*");
  auto hashes = redis.hgetall(keys);
  for(const auto& [key, fields]: hashes)
    cout << key << ": " << fields.size() << " fields" << endl;
}
catch (const Redis::Error& e)
{
  cerr << e.what() << endl;       // Will reconnect on the next call
}
```

## Build

```
NAME    = ot-redis
TYPE    = lib
DEPENDS = ot-net ot-mt ot-time
```

## License

Copyright (c) 2026 Paul Clark. MIT License.
