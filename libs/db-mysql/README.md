# ObTools::DB::MySQL

MySQL database driver implementing the ObTools DB abstraction layer.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-db-mysql.h"
using namespace ObTools;

DB::MySQL::Connection conn("localhost", "root", "password", "mydb");
if (!conn) return 1;

// Execute commands
conn.exec("CREATE TABLE IF NOT EXISTS users (id INT AUTO_INCREMENT PRIMARY KEY, name VARCHAR(256))");
conn.exec("INSERT INTO users (name) VALUES ('Alice')");
uint64_t id = conn.get_last_insert_id();

// Query
DB::Result res = conn.query("SELECT * FROM users");
DB::Row row;
while (res.fetch(row))
  cout << row["id"] << ": " << row["name"] << endl;
```

### Connection Pooling

```cpp
DB::MySQL::ConnectionFactory factory("localhost", "root", "password", "mydb");
DB::ConnectionPool pool(factory, 4);

auto conn = pool.claim();
conn->exec("INSERT INTO users (name) VALUES ('Bob')");
```

## Build

```
NAME    = ot-db-mysql
TYPE    = lib
DEPENDS = ot-db ext-mysqlclient
```

## License

Copyright (c) 2003 Paul Clark. MIT License.
