# CLAUDE.md - ObTools::DB Library

## Overview

`ObTools::DB` is an abstract database interface with typed fields, query building, result sets, prepared statements, connection pooling, and RAII transactions. Driver implementations are in separate libraries (`db-mysql`, `db-pgsql`, `db-sqlite`). Lives under `namespace ObTools::DB`.

**Header:** `ot-db.h`
**Dependencies:** `ot-time`, `ot-mt`, `ot-log`
**Platforms:** posix

## Key Classes

| Class | Purpose |
|-------|---------|
| `FieldValue` | Typed database value (null/string/int/int64/bool/real) |
| `Row` | Named field map with SQL generation |
| `ResultSet` | Abstract result set (cursor) |
| `Result` | RAII wrapper for ResultSet |
| `PreparedStatement` | Abstract prepared statement |
| `Statement` / `AutoStatement` | RAII wrappers for PreparedStatement |
| `Connection` | Abstract database connection with high-level CRUD |
| `ConnectionFactory` | Abstract factory for creating connections |
| `ConnectionPool` | Thread-safe connection pool with background cleanup |
| `AutoConnection` | RAII connection from pool |
| `Transaction` | RAII transaction (rollback on destruction if not committed) |

## FieldValue

```cpp
FieldValue();                           // null
FieldValue(const string& value);        // string
FieldValue(int value);                  // integer
FieldValue(uint64_t value);             // int64
FieldValue(bool value);                 // boolean
FieldValue(double value);               // real

FieldType get_type() const;
string as_string() const;
string as_quoted_string() const;        // escaped + quoted
int as_int() const;  uint64_t as_int64() const;
bool as_bool() const;  double as_real() const;
bool is_null() const;

static string escape(const string& s);   // double quotes and backslashes
static string unescape(const string& s);
static string quote(const string& s);    // escape + wrap in quotes
```

## Row

```cpp
Row& add(const string& field, const FieldValue& value);
Row& add(const string& field, const string& value);
Row& add(const string& field, int value);
Row& add_int64(const string& field, uint64_t value);
Row& add(const string& field, bool value);
Row& add(const string& field, double value);
void add_or_null(const string& field, const string& value);  // null if empty
void add_null(const string& field);
void add_time_or_null(const string& field, const Time::Stamp& value);
void add_date_or_null(const string& field, const Time::DateStamp& value);
Row& operator<<(const string& field);   // add null (for SELECT field lists)

bool has(const string& field) const;
string get(const string& field, const string& def="") const;
string operator[](const string& field) const;
int get_int(const string& field, int def=0) const;
uint64_t get_int64(const string& field, uint64_t def=0) const;
bool get_bool(const string& field, bool def=false) const;
double get_real(const string& field, double def=0) const;

// SQL generation
string get_fields() const;                  // "f1, f2, f3"
string get_escaped_values() const;          // "'v1', 'v2', 3"
string get_escaped_assignments() const;     // "f1='v1', f2=3"
string get_where_clause() const;            // "f1='v1' AND f2=3"
```

## Connection (abstract)

Drivers implement the virtual methods. The class provides high-level CRUD helpers.

```cpp
// Low-level (pure virtual)
virtual bool exec(const string& sql) = 0;
virtual Result query(const string& sql) = 0;
virtual Statement prepare(const string& sql) = 0;
virtual uint64_t get_last_insert_id() = 0;
virtual string utc_timestamp() = 0;

// Transactions
virtual bool transaction_begin();
virtual bool transaction_commit();
virtual bool transaction_rollback();

// Query helpers
bool query(const string& sql, Row& row);
bool query(const string& sql, string& value);
string query_string(const string& sql, const string& def="");
int query_int(const string& sql, int def=0);

// INSERT (returns generated ID)
int insert(const string& table, Row& row, const string& id_field="id");
uint64_t insert64(const string& table, Row& row, const string& id_field="id");
bool insert_or_update(const string& table, Row& row, Row& update_row);

// SELECT
Result select(const string& table, const Row& fields, const string& where="");
Result select(const string& table, const Row& fields, const Row& where_row);
Result select_by_id(const string& table, const Row& fields, int id);
bool select_row(const string& table, Row& row, const string& where="");
bool select_row_by_id(const string& table, Row& row, int id);
string select_value(const string& table, const string& field, const string& where="");

// COUNT / EXISTS
int count(const string& table, const string& where="");
bool exists_id(const string& table, int id);

// UPDATE
bool update(const string& table, const Row& row, const string& where="");
bool update_id(const string& table, const Row& row, int id);

// DELETE
bool delete_all(const string& table, const string& where="");
bool delete_id(const string& table, int id);

// All above also have 64-bit and string ID variants
```

## ConnectionPool

```cpp
ConnectionPool(ConnectionFactory& factory, unsigned min, unsigned max,
               Time::Duration max_inactivity);
Connection *claim();                           // blocks until available
void release(Connection *conn);
void set_reap_interval(const Time::Duration& i);
void set_claim_timeout(const Time::Duration& t);
unsigned num_connections() const;
unsigned num_available() const;
unsigned num_in_use() const;
```

## AutoConnection

RAII: claims on construction, releases on destruction.

```cpp
AutoConnection(ConnectionPool& pool);
explicit operator bool() const;
// All Connection methods proxied (return defaults if conn is null)
```

## Transaction

```cpp
Transaction(Connection& conn, bool immediate=false);
Transaction(AutoConnection& conn, bool immediate=false);
bool commit();
~Transaction();  // rollback if not committed
```

## File Layout

```
ot-db.h              - Public header
connection.cc        - Connection helpers
field-value.cc       - FieldValue implementation
row.cc               - Row SQL generation
pool.cc              - ConnectionPool
transaction.cc       - Transaction
test-pool.cc         - Pool tests (gtest)
test-row.cc          - Row tests
```
