//==========================================================================
// ObTools::DB: ot-db.h
//
// Public definitions for ObTools::DB Library
// General C++ database wrapper
// 
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_DB_H
#define __OBTOOLS_DB_H

#include <map>
#include <iostream>
#include <stdint.h>
#include "ot-time.h"

#if !defined(_SINGLE)
#include "ot-mt.h"
#endif

namespace ObTools { namespace DB { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// IMPORTANT

// Note on escaping quotes and backslashes:
//  1) All fields stored in the Row::fields map are _unescaped_
//  2) The driver unescapes all data as it fetches it
//  3) In raw operations such as exec() and query() it is up to the caller
//     to escape string values in the SQL
//  4) In 'cooked' operations such as insert() and select() where this 
//     library constructs its own SQL all values are escaped automatically

//==========================================================================
// Field type
enum FieldType
{
  NULLTYPE,
  STRING,
  INT,
  INT64,
  BOOL,
  REAL
};

//==========================================================================
// Field value
class FieldValue
{
private:
  // Value and type information
  string str_val;
  union
  {
    int int_val;
    uint64_t int64_val;
    bool bool_val;
    double real_val;
  };
  FieldType type;

public:
  //------------------------------------------------------------------------
  // Constructors
  FieldValue():
    type(NULLTYPE) {}

  FieldValue(const string& value):
    str_val(value), real_val(0), type(STRING) {}

  FieldValue(int value):
    int_val(value), type(INT) {}

  FieldValue(uint64_t value):
    int64_val(value), type(INT64) {}

  FieldValue(bool value):
    bool_val(value), type(BOOL) {}

  FieldValue(double value):
    real_val(value), type(REAL) {}

  //------------------------------------------------------------------------
  // Constructor from string but convert to specified type
  FieldValue(const string& value, FieldType _type);

  //------------------------------------------------------------------------
  // Get the type we think we are
  FieldType get_type() const
  {
    return type;
  }

  //------------------------------------------------------------------------
  // Get value as string
  string as_string() const;

  //------------------------------------------------------------------------
  // Get value as escaped string
  string as_escaped_string() const
  {
    if (type == STRING)
      return escape(as_string());
    else
      return as_string();
  }

  //------------------------------------------------------------------------
  // Get value as quoted escaped string
  string as_quoted_string() const
  {
    if (type == STRING)
      return quote(as_string());
    else
      return as_string();
  }

  //------------------------------------------------------------------------
  // Get value as int
  int as_int() const;

  //------------------------------------------------------------------------
  // Get value as int64
  uint64_t as_int64() const;

  //------------------------------------------------------------------------
  // Get value as bool
  bool as_bool() const;

  //------------------------------------------------------------------------
  // Get value as real
  double as_real() const;

  //------------------------------------------------------------------------
  // Is value/type null?
  bool is_null() const
  {
    return type == NULLTYPE;
  }

  //==========================================================================
  // Static helper functions

  //------------------------------------------------------------------------
  // Escape and quote a string
  static string quote(const string& s)
  {
    return "'" + escape(s) + "'";
  }

  //------------------------------------------------------------------------
  // Escape a string, doubling single quotes and backslashes
  static string escape(const string& s);

  //------------------------------------------------------------------------
  // Unescape a string, singling double quotes and backslashes
  static string unescape(const string& s);
};

//==========================================================================
// Database row (same for all drivers)
class Row
{
private:
  map<string, FieldValue> fields;

public:
  //------------------------------------------------------------------------
  //Clear the row
  void clear() 
  { fields.clear(); }

  //------------------------------------------------------------------------
  //Add name/value pair (from a FieldValue)
  Row& add(const string& fieldname, const FieldValue& value)
  { fields[fieldname] = value; return *this; }

  //------------------------------------------------------------------------
  // Handy operator to add null values to a row, for select()
  Row& operator<<(const string& fieldname) 
  { fields[fieldname] = FieldValue(); return *this; }

  //------------------------------------------------------------------------
  //Add name/value pair
  void add(const string& fieldname, const string& value)
  { fields[fieldname] = FieldValue(value); }

  //------------------------------------------------------------------------
  //Add name/value pair (NULL if empty)
  void add_or_null(const string& fieldname, const string& value)
  {
    if (value.size())
      fields[fieldname] = FieldValue(value);
    else
      fields[fieldname] = FieldValue();
  }

  //------------------------------------------------------------------------
  // Add a NULL entry
  void add_null(const string& fieldname)
  { fields[fieldname] = FieldValue(); }

  //------------------------------------------------------------------------
  //Add name/value pair, unescaping value (for use by drivers only)
  void add_unescaped(const string& fieldname, const string& value)
  { fields[fieldname] = FieldValue(FieldValue::unescape(value)); }

  //------------------------------------------------------------------------
  // Add integer value to row
  void add(string fieldname, int value)
  { fields[fieldname] = FieldValue(value); }

  //------------------------------------------------------------------------
  // Add integer value to row (NULL if zero)
  void add_or_null(string fieldname, int value)
  {
    if (value)
      fields[fieldname] = FieldValue(value);
    else
      fields[fieldname] = FieldValue();
  }

  //------------------------------------------------------------------------
  // Add a 64-bit integer value to row
  void add_int64(string fieldname, uint64_t value)
  { fields[fieldname] = FieldValue(value); }

  //------------------------------------------------------------------------
  // Add a 64-bit integer value to row (NULL if zero)
  void add_int64_or_null(string fieldname, uint64_t value)
  {
    if (value)
      fields[fieldname] = FieldValue(value);
    else
      fields[fieldname] = FieldValue();
  }

  //------------------------------------------------------------------------
  // Add boolean value to row
  void add(string fieldname, bool value)
  { fields[fieldname] = FieldValue(value); }

  //------------------------------------------------------------------------
  // Add real value to row
  void add(string fieldname, double value)
  { fields[fieldname] = FieldValue(value); }

  //------------------------------------------------------------------------
  // Add real value to row (NULL if zero)
  void add_or_null(string fieldname, double value)
  {
    if (value)
      fields[fieldname] = FieldValue(value);
    else
      fields[fieldname] = FieldValue();
  }

  //------------------------------------------------------------------------
  //Finds whether the row contains a value for the given fieldname
  bool has(string fieldname) const
  { return fields.find(fieldname) != fields.end(); }

  //------------------------------------------------------------------------
  //Get value of field of given name, or default if not found
  string get(string fieldname, const string& def="") const;

  //------------------------------------------------------------------------
  //Handy [] operator
  // e.g. foo = row["id"];
  string operator[](string fieldname) const
  { return get(fieldname); }

  //------------------------------------------------------------------------
  //Get integer value of field of given name, or default if not found
  int get_int(string fieldname, int def=0) const;

  //------------------------------------------------------------------------
  //Get 64-bit value of field of given name, or default if not found
  uint64_t get_int64(string fieldname, uint64_t def=0) const;

  //------------------------------------------------------------------------
  //Get boolean value of field of given name, or default if not found
  bool get_bool(string fieldname, bool def=false) const;

  //------------------------------------------------------------------------
  //Get real value of field of given name, or default if not found
  double get_real(string fieldname, double def=0) const;

  //------------------------------------------------------------------------
  // Get string with field names in order, separated by commas and spaces
  string get_fields() const;

  //------------------------------------------------------------------------
  // Get string with field values in order, separated by commas and spaces,
  // each escaped and delimited with single quotes (e.g. for INSERT)
  string get_escaped_values() const;

  //------------------------------------------------------------------------
  // Get string with field names and values in order with '=', 
  // separated by commas and spaces, values delimited with single quotes 
  // (e.g. for UPDATE)
  string get_escaped_assignments() const;

  //------------------------------------------------------------------------
  // Get string with field names and values in order with '=', separated
  // by AND, values delimited with single quotes
  // (e.g. for WHERE)
  string get_where_clause() const;
};

//==========================================================================
// Database result set (abstract)
class ResultSet
{
public:
  //------------------------------------------------------------------------
  //Get number of rows in result set
  virtual int count()=0;

  //------------------------------------------------------------------------
  //Get next row from result set
  //Whether another was found - if so, clears and writes into row
  virtual bool fetch(Row& row)=0;

  //------------------------------------------------------------------------
  //Get first value of next row from result set
  //Value is unescaped
  //Whether another was found - if so, writes into value
  virtual bool fetch(string& value)=0;

  //------------------------------------------------------------------------
  //Virtual destructor
  virtual ~ResultSet() {}
};

//==========================================================================
// Database result auto-ptr wrapper
// Allows inheritance of ResultSet while preserving single abstract interface
class Result
{
private:
  ResultSet *rset;

  // Helper to allow copy-by-value (see auto_ptr for source of this hack)
  struct Ref
  {
    ResultSet *rset;
    explicit Ref(ResultSet *r): rset(r) {}
  };

public:
  //------------------------------------------------------------------------
  //Constructors 
  Result(): rset(0) {}               // Invalid result
  Result(ResultSet *r): rset(r) {}   // Valid result

  //------------------------------------------------------------------------
  //Copy constructor and assignment operators
  // - detach rset from old one - note non-const
  Result(Result& r): rset(r.rset) { r.rset=0; }
  Result& operator=(Result& r) 
  { 
    if (rset && rset!=r.rset) delete rset;
    rset=r.rset; r.rset=0;
    return *this; 
  }

  //------------------------------------------------------------------------
  //Ditto from 'Ref' helper - combination of the two is claimed to allow
  //assignment by value - e.g.
  // Result res = my_func_returning_result();
  Result(Ref r): rset(r.rset) { }
  Result& operator=(Ref& r) 
  { 
    if (rset && rset!=r.rset) delete rset;
    rset=r.rset; 
    return *this; 
  }
  operator Ref() { ResultSet *t = rset; rset=0; return Ref(t); }

  //------------------------------------------------------------------------
  //Handy ! operator to check for (in)validity
  bool operator!() const { return !rset; }

  //------------------------------------------------------------------------
  //Get number of rows in result set
  int count() { return rset?rset->count():0; }

  //------------------------------------------------------------------------
  //Get next row from result set
  //Whether another was found - if so, clears writes into row
  bool fetch(Row& row) { return rset?rset->fetch(row):false; }

  //------------------------------------------------------------------------
  //Get first value of next row from result set
  //Whether another was found - if so, writes into value
  bool fetch(string& value) { return rset?rset->fetch(value):false; }

  //------------------------------------------------------------------------
  //Destructor - delete result set
  ~Result() { if (rset) delete rset; }
};

//==========================================================================
// Database connection (abstract)
class Connection
{
protected:
  bool valid;

public:
  //------------------------------------------------------------------------
  //Constructor
  Connection(): valid(false) {}

  //------------------------------------------------------------------------
  //Handy ! operator to check for (in)validity
  bool operator!() const { return !valid; }

  //==========================================================================
  // Virtual functions implemented by driver subclass

  //------------------------------------------------------------------------
  //Check if connection is really OK
  virtual bool ok()=0;

  //------------------------------------------------------------------------
  //Execute a command, not expecting any result (e.g. INSERT, UPDATE, DELETE)
  //Returns whether successful
  virtual bool exec(const string& sql)=0;

  //------------------------------------------------------------------------
  //Execute a query and get result (e.g. SELECT)
  //Returns result - check this for validity
  virtual Result query(const string& sql)=0;

  //------------------------------------------------------------------------
  //Virtual destructor
  virtual ~Connection() {}

  //==========================================================================
  // Helper functions implemented in connection.cc

  //------------------------------------------------------------------------
  //Execute a query and get first (only) row
  //Returns whether successful - row is cleared and filled in if so
  bool query(const string& sql, Row& row);

  //------------------------------------------------------------------------
  //Execute a query and get single (only) value from first (only) row
  //Returns whether successful - value is filled in if so
  bool query(const string& sql, string& value);

  //------------------------------------------------------------------------
  //Execute a query and get single (only) value from first (only) row
  //Returns value or default if not found
  string query_string(const string& sql, const string& def="");

  //------------------------------------------------------------------------
  //Execute a query and get single (only) integer value from first (only) row
  //Returns value or default if not found
  int query_int(const string& sql, int def=0);

  //------------------------------------------------------------------------
  //Execute a query and get single (only) 64-bit integer value from first 
  //(only) row
  //Returns value or default if not found
  uint64_t query_int64(const string& sql, uint64_t def=0);

  //------------------------------------------------------------------------
  //Execute a query and get single (only) integer value from first (only) row
  //Returns value or default if not found
  bool query_bool(const string& sql, bool def=false);

  //------------------------------------------------------------------------
  //Do an INSERT and retrieve the last inserted (max) automatic ID
  //Returns ID, or 0 if failed
  //Fetches max(<id_field>) inside a transaction, unless id_field is ""
  //If id_field is "", does a normal insert and returns 1
  //Set in_transaction only if you're already doing a transaction; by
  //default this function may create its own
  int insert(const string& sql, 
	     const string& table, const string& id_field="id",
	     bool in_transaction=false);

  // Ditto, with 64-bit ID
  uint64_t insert64(const string& sql, 
		    const string& table, const string& id_field="id",
		    bool in_transaction=false);

  //------------------------------------------------------------------------
  // Do an INSERT and retrieve the last inserted serial ID, from row data
  // Each field in the row is inserted by name
  // id_field can be "", as above
  // Returns ID, or 0 if failed
  int insert(const string& table, Row& row, const string& id_field="id",
	     bool in_transaction=false);

  // Ditto with 64-bit ID
  uint64_t insert64(const string& table, Row& row, const string& id_field="id",
		    bool in_transaction=false);

  //------------------------------------------------------------------------
  // INSERT into a join table with two foreign ID fields
  // Returns whether successful
  bool insert_join(const string& table, const string& field1, int id1,
		   const string& field2, int id2);

  // Ditto with 64-bit IDs
  bool insert_join64(const string& table, const string& field1, uint64_t id1,
		     const string& field2, uint64_t id2);

  //------------------------------------------------------------------------
  // Do a SELECT for all fields in the given row in the given table 
  // with the given WHERE clause
  // If where is empty, doesn't add a WHERE at all
  // Returns query result as query()
  Result select(const string& table, const Row& row, const string& where="");

  //------------------------------------------------------------------------
  // Do a SELECT for all fields in the given row in the given table 
  // matching the list of values in where_row
  // Returns query result as query()
  Result select(const string& table, const Row& row, const Row& where_row);

  //------------------------------------------------------------------------
  // Do a SELECT for all fields in the given row in the given table 
  // matching the given integer ID
  // Returns query result as query()
  Result select_by_id(const string& table, const Row& row,  
		      int id, const string& id_field = "id");

  // Ditto, with 64-bit ID
  Result select_by_id64(const string& table, const Row& row,  
			uint64_t id, const string& id_field = "id");

  //------------------------------------------------------------------------
  // Do a SELECT for all fields in the given row in the given table 
  // matching the given string ID
  // ID value is escaped
  // Returns query result as query()
  Result select_by_id(const string& table, const Row& row,  
		      const string& id, const string& id_field = "id");

  //------------------------------------------------------------------------
  // Do a SELECT for all fields in the given row in the given table 
  // with the given WHERE clause, and return the single (first) row as
  // the values in the row (unescaped)
  // If where is empty, doesn't add a WHERE at all
  // Returns whether row fetched
  bool select_row(const string& table, Row& row, const string& where="");

  //------------------------------------------------------------------------
  // Do a SELECT for all fields in the given row in the given table 
  // with a WHERE clause constructed from where_row, and return the single 
  // (first) row as the values in the row
  // Returns whether row fetched
  bool select_row(const string& table, Row& row, const Row& where_row);

  //------------------------------------------------------------------------
  // Do a SELECT for all fields in the given row in the given table 
  // with the given integer ID, and return the single (first) row as
  // the values in the row (unescaped)
  // Returns whether row fetched
  bool select_row_by_id(const string& table, Row& row, 
			int id, const string& id_field = "id");

  // Ditto, with 64-bit ID
  bool select_row_by_id64(const string& table, Row& row, 
			  uint64_t id, const string& id_field = "id");

  //------------------------------------------------------------------------
  // Do a SELECT for all fields in the given row in the given table 
  // with the given string ID, and return the single (first) row as
  // the values in the row (unescaped)
  // ID value is escaped
  // Returns whether row fetched
  bool select_row_by_id(const string& table, Row& row, 
			const string& id, const string& id_field = "id");

  //------------------------------------------------------------------------
  // Do a SELECT for a single field in the given table 
  // with the given WHERE clause, and return the (unescaped) value
  // If where is empty, doesn't add a WHERE at all
  // Returns value or empty string if not found
  string select_value(const string& table, const string& field,
		      const string& where = "");

  //------------------------------------------------------------------------
  // Do a SELECT for a single field in the given table 
  // with a WHERE clause constructed from where_row, and return the 
  // (unescaped) value
  // Returns value or empty string if not found
  string select_value(const string& table, const string& field,
		      const Row& where_row);

  //------------------------------------------------------------------------
  // Do a SELECT for a single field in the given table 
  // with the given integer ID, and return the (unescaped) value
  // Returns value or empty string if not found
  string select_value_by_id(const string& table, 
			    const string& field,
			    int id, const string& id_field = "id");

  // Ditto, with 64-bit ID
  string select_value_by_id64(const string& table, 
			      const string& field,
			      uint64_t id, const string& id_field = "id");

  //------------------------------------------------------------------------
  // Do a SELECT for a single field in the given table 
  // with the given string ID, and return the (unescaped) value
  // ID value is escaped
  // Returns value or empty string if not found
  string select_value_by_id(const string& table, const string& field,
			    const string& id, const string& id_field = "id");

  //------------------------------------------------------------------------
  // Count rows in the given table with the given WHERE clause
  // If where is empty, doesn't add a WHERE at all
  int count(const string& table, const string& where="");

  //------------------------------------------------------------------------
  // Count rows in the given table matching the list of values in where_row
  int count(const string& table, const Row& where_row);

  //------------------------------------------------------------------------
  // Check if a row exists with the given integer ID
  // Returns whether the row exists
  bool exists_id(const string& table, int id, const string& id_field = "id");

  // Ditto, with 64-bit ID
  bool exists_id64(const string& table, uint64_t id, 
		   const string& id_field = "id");

  //------------------------------------------------------------------------
  // Check if a row exists with the given string ID
  // ID value is escaped
  // Returns whether the row exists
  bool exists_id(const string& table, const string& id, 
		 const string& id_field = "id");

  //------------------------------------------------------------------------
  // Do an UPDATE for all fields in the given row in the given table 
  // with the given WHERE clause.  Values are escaped automatically
  // If where is empty, doesn't add a WHERE at all
  // Returns whether successful
  bool update(const string& table, const Row& row, const string& where="");

  //------------------------------------------------------------------------
  // Do an UPDATE for all fields in the given row in the given table 
  // with a WHERE clause created from where_row.  
  // Values are escaped automatically
  // Returns whether successful
  bool update(const string& table, const Row& row, const Row& where_row);

  //------------------------------------------------------------------------
  // Do an UPDATE for all fields in the given row in the given table 
  // matching the given integer ID
  // Returns whether successful
  bool update_id(const string& table, const Row& row, 
		 int id, const string& id_field = "id");

  // Ditto with 64-bit ID
  bool update_id64(const string& table, const Row& row, 
		   uint64_t id, const string& id_field = "id");

  //------------------------------------------------------------------------
  // Do an UPDATE for all fields in the given row in the given table 
  // matching the given string ID
  // ID value is escaped
  // Returns whether successful
  bool update_id(const string& table, const Row& row, 
		 const string& id, const string& id_field = "id");

  //------------------------------------------------------------------------
  // Do a DELETE in the given table with the given WHERE clause
  // If where is empty, doesn't add a WHERE at all
  // Returns whether successful
  bool delete_all(const string& table, const string& where = "");

  //------------------------------------------------------------------------
  // Do a DELETE in the given table with a WHERE clause created from where_row
  // Returns whether successful
  bool delete_all(const string& table, const Row& where_row);

  //------------------------------------------------------------------------
  // Do an DELETE in the given table matching the given integer ID
  // Returns whether successful
  bool delete_id(const string& table, 
		 int id, const string& id_field = "id");

  // Ditto with 64-bit ID
  bool delete_id64(const string& table, 
		   uint64_t id, const string& id_field = "id");

  //------------------------------------------------------------------------
  // Do a DELETE in the given table matching the given string ID
  // ID value is escaped
  // Returns whether successful
  bool delete_id(const string& table, 
		 const string& id, const string& id_field = "id");

  //------------------------------------------------------------------------
  // DELETE from a join table with two foreign ID fields
  // Returns whether successful
  bool delete_join(const string& table, 
		   const string& field1, int id1,
		   const string& field2, int id2);

  // Ditto with 64-bit ID
  bool delete_join64(const string& table, 
		     const string& field1, uint64_t id1,
		     const string& field2, uint64_t id2);

};

//==========================================================================
// Database connection factory - abstract interface implemented in driver
// subclasses.  Factories will store the connection details and create
// a new connection with them on demand
class ConnectionFactory
{
public:
  //------------------------------------------------------------------------
  // Constructor
  ConnectionFactory() {}

  //------------------------------------------------------------------------
  // Interface to create a new connection 
  virtual Connection *create() = 0;

  //------------------------------------------------------------------------
  // Virtual destructor
  virtual ~ConnectionFactory() {}
};

//==========================================================================
// Database connection pool - maintains list of database connections which
// can be claimed and released
class ConnectionPool
{
  ConnectionFactory& factory;
  unsigned min_connections;        // Number started on creation
  unsigned max_connections;        // Maximum ever created
  Time::Duration max_inactivity;

  // Current connections
  MT::Mutex mutex;                 // On connection lists
  list<Connection *> connections;  // All connections
  list<Connection *> available;    // Connections available for use

  // Background thread and timestamps
  map<Connection *, Time::Stamp> last_used;
  MT::Thread *background_thread;

  // Internals
  void fill_to_minimum();

public:
  //------------------------------------------------------------------------
  // Constructor
  ConnectionPool(ConnectionFactory& _factory, unsigned _min, unsigned _max,
		 Time::Duration _max_inactivity);

  //------------------------------------------------------------------------
  // Claim a connection
  // Returns connection, or 0 if one could not be created or all are active
  Connection *claim();

  //------------------------------------------------------------------------
  // Release a connection after use
  void release(Connection *conn);

  //------------------------------------------------------------------------
  // Run background timeout loop (called from internal thread)
  void run_background();

  //------------------------------------------------------------------------
  // Destructor
  ~ConnectionPool();
};

//==========================================================================
// Handy auto class to claim/release a connection and provide stubs to
// using it
// e.g.
// {
//   DB::AutoConnection conn(db_pool);
//   Result r = conn.query(...);
//   ...
// }
struct AutoConnection
{
  ConnectionPool& pool;
  Connection *conn;

  //------------------------------------------------------------------------
  // Constructor from pool
  AutoConnection(ConnectionPool& _pool): pool(_pool)
  { conn = pool.claim(); }

  //------------------------------------------------------------------------
  // Check if connection is valid
  bool ok() { return conn!=0; }
  bool operator!() { return !conn; }

  //------------------------------------------------------------------------
  // Stub functions for every connection operation - see above
  bool exec(const string& sql) { return conn?conn->exec(sql):false; }

  Result query(const string& sql) { return conn?conn->query(sql):Result(); }

  bool query(const string& sql, Row& row) 
  { return conn?conn->query(sql, row):false; }

  bool query(const string& sql, string& value)
  { return conn?conn->query(sql, value):false; }

  string query_string(const string& sql, const string& def="")
  { return conn?conn->query_string(sql, def):def; }

  int query_int(const string& sql, int def=0)
  { return conn?conn->query_int(sql, def):def; }

  uint64_t query_int64(const string& sql, uint64_t def=0)
  { return conn?conn->query_int64(sql, def):def; }

  bool query_bool(const string& sql, bool def=false)
  { return conn?conn->query_bool(sql, def):def; }

  int insert(const string& sql, 
	     const string& table, const string& id_field="id",
	     bool in_transaction=false)
  { return conn?conn->insert(sql, table, id_field, in_transaction):0; }

  uint64_t insert64(const string& sql, 
		    const string& table, const string& id_field="id",
		    bool in_transaction=false)
  { return conn?conn->insert64(sql, table, id_field, in_transaction):0; }

  int insert(const string& table, Row& row, const string& id_field="id",
	     bool in_transaction=false)
  { return conn?conn->insert(table, row, id_field, in_transaction):0; }

  bool insert_join(const string& table, const string& field1, int id1,
		   const string& field2, int id2)
  { return conn?conn->insert_join(table, field1, id1, field2, id2):false; }

  bool insert_join64(const string& table, const string& field1, uint64_t id1,
		     const string& field2, uint64_t id2)
  { return conn?conn->insert_join64(table, field1, id1, field2, id2):false; }

  Result select(const string& table, const Row& row, const string& where="")
  { return conn?conn->select(table, row, where):Result(); }

  Result select(const string& table, const Row& row, const Row& where_row)
  { return conn?conn->select(table, row, where_row):Result(); }

  Result select_by_id(const string& table, const Row& row,  
		      int id, const string& id_field = "id")
  { return conn?conn->select_by_id(table, row, id, id_field):Result(); }

  Result select_by_id64(const string& table, const Row& row,  
			uint64_t id, const string& id_field = "id")
  { return conn?conn->select_by_id64(table, row, id, id_field):Result(); }

  Result select_by_id(const string& table, const Row& row,  
		      const string& id, const string& id_field = "id")
  { return conn?conn->select_by_id(table, row, id, id_field):Result(); }

  bool select_row(const string& table, Row& row, const string& where="")
  { return conn?conn->select_row(table, row, where):false; }

  bool select_row(const string& table, Row& row, const Row& where_row)
  { return conn?conn->select_row(table, row, where_row):false; }

  bool select_row_by_id(const string& table, Row& row, 
			int id, const string& id_field = "id")
  { return conn?conn->select_row_by_id(table, row, id, id_field):false; }

  bool select_row_by_id64(const string& table, Row& row, 
			  uint64_t id, const string& id_field = "id")
  { return conn?conn->select_row_by_id64(table, row, id, id_field):false; }

  bool select_row_by_id(const string& table, Row& row, 
			const string& id, const string& id_field = "id")
  { return conn?conn->select_row_by_id(table, row, id, id_field):false; }

  string select_value(const string& table, const string& field,
		      const string& where = "")
  { return conn?conn->select_value(table, field, where):""; }

  string select_value(const string& table, const string& field,
		      const Row& where_row)
  { return conn?conn->select_value(table, field, where_row):""; }

  string select_value_by_id(const string& table, 
			    const string& field,
			    int id, const string& id_field = "id")
  { return conn?conn->select_value_by_id(table, field, id, id_field):""; }

  string select_value_by_id64(const string& table, 
			      const string& field,
			      uint64_t id, const string& id_field = "id")
  { return conn?conn->select_value_by_id64(table, field, id, id_field):""; }

  string select_value_by_id(const string& table, const string& field,
			    const string& id, const string& id_field = "id")
  { return conn?conn->select_value_by_id(table, field, id, id_field):""; }

  int count(const string& table, const string& where="")
  { return conn?conn->count(table, where):0; }

  int count(const string& table, const Row& where_row)
  { return conn?conn->count(table, where_row):0; }

  bool exists_id(const string& table, int id, const string& id_field = "id")
  { return conn?conn->exists_id(table, id, id_field):false; }

  bool exists_id64(const string& table, uint64_t id, 
		   const string& id_field = "id")
  { return conn?conn->exists_id64(table, id, id_field):false; }

  bool exists_id(const string& table, const string& id, 
		 const string& id_field = "id")
  { return conn?conn->exists_id(table, id, id_field):false; }

  bool update(const string& table, const Row& row, const string& where="")
  { return conn?conn->update(table, row, where):false; }

  bool update(const string& table, const Row& row, const Row& where_row)
  { return conn?conn->update(table, row, where_row):false; }

  bool update_id(const string& table, const Row& row, 
		 int id, const string& id_field = "id")
  { return conn?conn->update_id(table, row, id, id_field):false; }

  bool update_id64(const string& table, const Row& row, 
		   uint64_t id, const string& id_field = "id")
  { return conn?conn->update_id64(table, row, id, id_field):false; }

  bool update_id(const string& table, const Row& row, 
		 const string& id, const string& id_field = "id")
  { return conn?conn->update_id(table, row, id, id_field):false; }

  bool delete_all(const string& table, const string& where = "")
  { return conn?conn->delete_all(table, where):false; }

  bool delete_all(const string& table, const Row& where_row)
  { return conn?conn->delete_all(table, where_row):false; }

  bool delete_id(const string& table, 
		 int id, const string& id_field = "id")
  { return conn?conn->delete_id(table, id, id_field):false; }

  bool delete_id64(const string& table, 
		   uint64_t id, const string& id_field = "id")
  { return conn?conn->delete_id64(table, id, id_field):false; }

  bool delete_id(const string& table, 
		 const string& id, const string& id_field = "id")
  { return conn?conn->delete_id(table, id, id_field):false; }

  bool delete_join(const string& table, const string& field1, int id1,
		   const string& field2, int id2)
  { return conn?conn->delete_join(table, field1, id1, field2, id2):false; }

  bool delete_join64(const string& table, const string& field1, uint64_t id1,
		   const string& field2, uint64_t id2)
  { return conn?conn->delete_join64(table, field1, id1, field2, id2):false; }

  static string escape(const string& s) { return FieldValue::escape(s); }
  static string unescape(const string& s) { return FieldValue::unescape(s); }

  //------------------------------------------------------------------------
  // Destructor
  ~AutoConnection() { if (conn) pool.release(conn); }
};

//==========================================================================
// Transaction - provide transaction on given connection while it exists,
// rolls back if killed without commit
class Transaction
{
private:
  Connection *conn;
  bool committed;

public:
  //------------------------------------------------------------------------
  //Constructor from plain connection
  Transaction(Connection& _conn);

  //------------------------------------------------------------------------
  //Constructor from AutoConnection
  Transaction(AutoConnection& _conn);

  //------------------------------------------------------------------------
  //Commit - returns whether commit command ran OK
  bool commit();

  //------------------------------------------------------------------------
  //Destructor
  ~Transaction();
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_DB_H



