# ObTools::DB::PG

PostgreSQL database driver implementing the ObTools DB abstraction layer using libpq.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-db-pgsql.h"
using namespace ObTools;

DB::PG::Connection conn("host=localhost dbname=mydb user=postgres password=secret");
if (!conn) return 1;

// Execute commands
conn.exec("CREATE TABLE IF NOT EXISTS users (id SERIAL PRIMARY KEY, name VARCHAR(256))");
conn.exec("INSERT INTO users (name) VALUES ('Alice')");

// Query
DB::Result res = conn.query("SELECT * FROM users");
DB::Row row;
while (res.fetch(row))
  cout << row["id"] << ": " << row["name"] << endl;
```

### Connection Pooling

```cpp
DB::PG::ConnectionFactory factory("host=localhost dbname=mydb user=postgres");
DB::ConnectionPool pool(factory, 4);

auto conn = pool.claim();
conn->exec("INSERT INTO users (name) VALUES ('Bob')");
```

## Build

```
NAME    = ot-db-pgsql
TYPE    = lib
DEPENDS = ot-db ext-pkg-libpq
```

## License

Copyright (c) 2003 Paul Clark. MIT License.
