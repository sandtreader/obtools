# CLAUDE.md - ObTools::DB::SQLite Library

## Overview

`ObTools::DB::SQLite` is an SQLite3 database wrapper implementing the ObTools DB abstraction layer. Lives under `namespace ObTools::DB::SQLite`.

**Header:** `ot-db-sqlite.h`
**Dependencies:** `ot-db`, `ot-file`, `ext-pkg-sqlite3`
**Platforms:** Linux only

## Key Classes

| Class | Purpose |
|-------|---------|
| `Connection` | SQLite database connection |
| `PreparedStatement` | Prepared statement with parameter binding |
| `ResultSet` | Query result set (extends PreparedStatement) |
| `ConnectionFactory` | Factory for connection pooling |

## Connection

```cpp
Connection(const string& filename, const Time::Duration& timeout);
explicit operator bool() override;
bool exec(const string& sql) override;
Result query(const string& sql) override;
Statement prepare(const string& sql) override;
uint64_t get_last_insert_id() override;
bool insert_or_update(const string& table, Row& row, Row& update_row) override;
string utc_timestamp() override;              // "datetime('now')"
```

## PreparedStatement

```cpp
// Bind parameters (1-indexed)
bool bind(int index, bool value);
bool bind(int index, int64_t value);
bool bind(int index, uint64_t value);
bool bind(int index, double value);
bool bind(int index, const string& value);
bool bind(int index);                          // NULL

// Execute and fetch
bool execute();
bool fetch(Row& row);
bool fetch(string& value);
bool next();
void reset();

// Column access
string get_string(int col);
uint64_t get_int(int col);
double get_real(int col);
Time::Stamp get_time(int col);
```

## ConnectionFactory

```cpp
ConnectionFactory(const string& _file, const Time::Duration& _timeout);
ConnectionFactory(const string& _file, const Time::Duration& _timeout,
                  const map<string, string>& statements);
```
