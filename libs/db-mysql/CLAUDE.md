# CLAUDE.md - ObTools::DB::MySQL Library

## Overview

`ObTools::DB::MySQL` is a MySQL database driver implementing the ObTools DB abstraction layer. Lives under `namespace ObTools::DB::MySQL`.

**Header:** `ot-db-mysql.h`
**Dependencies:** `ot-db`, `ext-mysqlclient`
**Platforms:** Linux only

## Key Classes

| Class | Purpose |
|-------|---------|
| `Connection` | MySQL database connection |
| `ResultSet` | Query result set with row iteration |
| `ConnectionFactory` | Factory for connection pooling |

## Connection

```cpp
Connection(const string& host, const string& user, const string& passwd,
           const string& dbname, unsigned int port = 0);
explicit operator bool() override;                  // mysql_ping()
bool exec(const string& sql) override;
Result query(const string& sql) override;
Statement prepare(const string&) override;          // throws (not implemented)
uint64_t get_last_insert_id() override;
string utc_timestamp() override;                    // "utc_timestamp()"
```

## ResultSet

```cpp
ResultSet(MYSQL_RES *res);
int count() override;
bool fetch(Row& row) override;         // field name -> value map
bool fetch(string& value) override;    // first column only
```

## ConnectionFactory

```cpp
ConnectionFactory(const string& host, const string& user,
                  const string& passwd, const string& dbname,
                  unsigned int port = 0);
ConnectionFactory(const string& host, const string& user,
                  const string& passwd, const string& dbname,
                  unsigned int port, const map<string, string>& statements);
```
