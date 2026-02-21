# CLAUDE.md - ObTools::DB::PG Library

## Overview

`ObTools::DB::PG` is a PostgreSQL database driver implementing the ObTools DB abstraction layer using libpq. Lives under `namespace ObTools::DB::PG`.

**Header:** `ot-db-pgsql.h`
**Dependencies:** `ot-db`, `ext-pkg-libpq`
**Platforms:** Linux only

## Key Classes

| Class | Purpose |
|-------|---------|
| `Connection` | PostgreSQL database connection |
| `ResultSet` | Query result set with row iteration |
| `ConnectionFactory` | Factory for connection pooling |

## Connection

```cpp
Connection(const string& conninfo);     // e.g., "host=localhost dbname=foo user=prc"
explicit operator bool() override;       // PQstatus() == CONNECTION_OK
bool exec(const string& sql) override;
Result query(const string& sql) override;
Statement prepare(const string&) override;          // throws (not implemented)
uint64_t get_last_insert_id() override;             // throws (not supported)
string utc_timestamp() override;                     // "current_timestamp at time zone 'UTC'"
```

## ResultSet

```cpp
ResultSet(PGresult *res);
int count() override;                    // PQntuples()
bool fetch(Row& row) override;          // field name -> value map
bool fetch(string& value) override;     // first column only
```

## ConnectionFactory

```cpp
ConnectionFactory(const string& conninfo);
ConnectionFactory(const string& conninfo, const map<string, string>& statements);
```
