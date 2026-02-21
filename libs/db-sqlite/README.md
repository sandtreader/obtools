# ObTools::DB::SQLite

SQLite3 database wrapper implementing the ObTools DB abstraction layer, with prepared statements, connection pooling, and transaction support.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-db-sqlite.h"
using namespace ObTools;

// Open database with 5-second busy timeout
DB::SQLite::Connection conn("app.db", Time::Duration(5.0));
if (!conn) return 1;

// Create table and insert data
conn.exec("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT)");
conn.exec("INSERT INTO users (name) VALUES ('Alice')");
uint64_t id = conn.get_last_insert_id();

// Query
DB::Result res = conn.query("SELECT * FROM users");
DB::Row row;
while (res.fetch(row))
  cout << row["id"] << ": " << row["name"] << endl;
```

### Prepared Statements

```cpp
auto stmt = conn.prepare("INSERT INTO users (name) VALUES (?)");
stmt.bind(1, "Bob");
stmt.execute();
stmt.reset();
stmt.bind(1, "Charlie");
stmt.execute();
```

### Connection Pooling

```cpp
DB::SQLite::ConnectionFactory factory("app.db", Time::Duration(5.0));
DB::ConnectionPool pool(factory, 4);  // 4 connections

auto conn = pool.claim();
conn->exec("INSERT INTO users (name) VALUES ('Dave')");
```

## Build

```
NAME    = ot-db-sqlite
TYPE    = lib
DEPENDS = ot-db ot-file ext-pkg-sqlite3
```

## License

Copyright (c) 2014 Paul Clark. MIT License.
