# ObTools::DB

An abstract database interface for C++17 with typed fields, query building, result sets, prepared statements, connection pooling, and RAII transactions. Database drivers (`db-mysql`, `db-pgsql`, `db-sqlite`) provide concrete implementations.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Features

- **Abstract interface**: write code once, swap database backends
- **Typed fields**: FieldValue handles null/string/int/int64/bool/real
- **Query building**: Row generates escaped SQL for INSERT/UPDATE/WHERE
- **Prepared statements**: parameterised queries with bind/fetch
- **Connection pooling**: thread-safe pool with background cleanup
- **RAII patterns**: Result, Statement, AutoConnection, Transaction
- **High-level CRUD**: select/insert/update/delete helpers on Connection

## Dependencies

- `ot-time` - Timestamp handling
- `ot-mt` - Multithreading (for connection pool)
- `ot-log` - Logging

## Quick Start

```cpp
#include "ot-db.h"
// Plus a driver header, e.g.:
// #include "ot-db-pgsql.h" or "ot-db-mysql.h" or "ot-db-sqlite.h"
using namespace ObTools;
```

### Building Rows

```cpp
// For INSERT/UPDATE
DB::Row row;
row.add("name", "Alice");
row.add("age", 30);
row.add("active", true);
row.add("score", 95.5);
row.add_int64("big_id", 123456789012345ULL);
row.add_or_null("email", "");           // NULL if empty
row.add_null("deleted_at");
row.add_time_or_null("created_at", Time::Stamp::now());

// For SELECT (field list)
DB::Row fields;
fields << "name" << "age" << "email";

// For WHERE
DB::Row where;
where.add("active", true);
where.add("age", 30);
```

### Reading Row Results

```cpp
DB::Row row;
// ... fetched from query ...
string name = row.get("name", "unknown");
string name2 = row["name"];              // no default
int age = row.get_int("age");
uint64_t id = row.get_int64("big_id");
bool active = row.get_bool("active");
double score = row.get_real("score");
bool has_email = row.has("email");
```

### Direct Connection

```cpp
// Create connection via driver (example with SQLite)
// DB::SQLite::Connection conn("mydb.sqlite");

DB::Connection& conn = /* driver-specific connection */;

// Raw SQL
conn.exec("CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT, age INT)");

// INSERT with auto-ID
DB::Row row;
row.add("name", "Alice");
row.add("age", 30);
int id = conn.insert("users", row);  // returns generated ID

// SELECT
DB::Row fields;
fields << "name" << "age";
DB::Result result = conn.select("users", fields, "age > 25");
DB::Row r;
while (result.fetch(r))
  cout << r["name"] << ": " << r.get_int("age") << endl;

// SELECT single row
DB::Row user;
user << "name" << "age";
if (conn.select_row_by_id("users", user, 1))
  cout << user["name"] << endl;

// SELECT single value
string name = conn.select_value("users", "name", "id=1");

// UPDATE
DB::Row updates;
updates.add("age", 31);
conn.update_id("users", updates, 1);

// DELETE
conn.delete_id("users", 1);

// COUNT / EXISTS
int n = conn.count("users", "age > 25");
bool exists = conn.exists_id("users", 1);
```

### Row-Based WHERE

```cpp
DB::Row where;
where.add("active", true);
where.add("age", 30);

DB::Row fields;
fields << "name" << "email";

// SELECT ... WHERE active='true' AND age=30
DB::Result result = conn.select("users", fields, where);

// UPDATE ... WHERE active='true' AND age=30
DB::Row updates;
updates.add("status", "verified");
conn.update("users", updates, where);

// DELETE ... WHERE active='true' AND age=30
conn.delete_all("users", where);
```

### INSERT or UPDATE (Upsert)

```cpp
DB::Row row;
row.add("id", 1);
row.add("name", "Alice");
row.add("age", 31);

DB::Row update_row;
update_row.add("name", "Alice");
update_row.add("age", 31);

conn.insert_or_update("users", row, update_row);
```

### Prepared Statements

```cpp
DB::Statement stmt = conn.prepare("SELECT name, age FROM users WHERE id = ?");
stmt.bind(1, 42);         // bind parameter 1 to int 42
stmt.execute();

while (stmt.next())
{
  string name = stmt.get_string(0);
  uint64_t age = stmt.get_int(1);
}

// Held statements (prepared once, reused)
conn.prepare_statement("get_user", "SELECT name FROM users WHERE id = ?");
auto s = conn.get_statement("get_user");
s.bind(1, 42);
s.execute();
// AutoStatement resets on destruction
```

### Connection Pooling

```cpp
// Create factory (driver-specific)
// MyDriverFactory factory(connection_params);

DB::ConnectionPool pool(factory,
  2,                          // min connections
  10,                         // max connections
  Time::Duration("5 min")     // max inactivity before reap
);

pool.set_claim_timeout(Time::Duration("30 sec"));

// RAII connection claim
{
  DB::AutoConnection conn(pool);
  if (!conn) { /* pool exhausted */ }

  conn.exec("INSERT INTO logs (msg) VALUES ('hello')");
  string val = conn.query_string("SELECT COUNT(*) FROM logs");
}  // connection returned to pool
```

### Transactions

```cpp
{
  DB::AutoConnection conn(pool);
  DB::Transaction tx(conn);  // BEGIN

  conn.exec("UPDATE accounts SET balance = balance - 100 WHERE id = 1");
  conn.exec("UPDATE accounts SET balance = balance + 100 WHERE id = 2");

  tx.commit();  // COMMIT
}  // if not committed, destructor does ROLLBACK

// Immediate transaction (SQLite)
DB::Transaction tx(conn, true);  // BEGIN IMMEDIATE
```

### FieldValue

```cpp
DB::FieldValue v("hello");
v.get_type();              // DB::STRING
v.as_string();             // "hello"
v.as_quoted_string();      // "'hello'"

DB::FieldValue n(42);
n.as_int();                // 42
n.as_string();             // "42"

// Escaping
string safe = DB::FieldValue::escape("it's \"quoted\"");
string quoted = DB::FieldValue::quote("it's");  // "'it''s'"
```

## API Reference

### FieldValue

| Method | Returns | Description |
|--------|---------|-------------|
| `get_type()` | `FieldType` | NULLTYPE/STRING/INT/INT64/BOOL/REAL |
| `as_string/int/int64/bool/real()` | typed | Convert to type |
| `as_quoted_string()` | `string` | Escaped and quoted |
| `is_null()` | `bool` | Null check |

### Row

| Method | Description |
|--------|-------------|
| `add(field, value)` | Add typed field |
| `add_or_null(field, value)` | Add or NULL if empty/zero |
| `add_null(field)` | Add explicit NULL |
| `get/get_int/get_bool/get_real(field)` | Read typed field |
| `get_fields()` | SQL field list |
| `get_escaped_values()` | SQL value list |
| `get_escaped_assignments()` | SQL SET clause |
| `get_where_clause()` | SQL WHERE clause |

### Connection

| Method | Description |
|--------|-------------|
| `exec(sql)` | Execute non-query SQL |
| `query(sql)` | Execute query, return Result |
| `prepare(sql)` | Create prepared statement |
| `insert(table, row)` | INSERT, return ID |
| `select(table, fields, where)` | SELECT with WHERE |
| `select_row_by_id(table, row, id)` | SELECT single row |
| `update_id(table, row, id)` | UPDATE by ID |
| `delete_id(table, id)` | DELETE by ID |
| `count(table, where)` | COUNT rows |
| `exists_id(table, id)` | EXISTS check |

### Pool / Transaction

| Class | Key Methods |
|-------|-------------|
| `ConnectionPool` | `claim()`, `release()`, `num_available()`, `num_in_use()` |
| `AutoConnection` | RAII claim/release, proxies all Connection methods |
| `Transaction` | `commit()`, destructor rolls back if uncommitted |

## Build

```
NAME      = ot-db
TYPE      = lib
DEPENDS   = ot-time ot-mt ot-log
PLATFORMS = posix
```

## License

Copyright (c) 2003 Paul Clark. MIT License.
