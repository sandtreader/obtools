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

// Make our lives easier without polluting anyone else
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
    uint64_t int64_val = 0;
    int int_val;
    bool bool_val;
    double real_val;
  };
  FieldType type;

public:
  //------------------------------------------------------------------------
  // Constructors
  FieldValue():
    type{NULLTYPE} {}

  FieldValue(const string& value):
    str_val{value}, type{STRING} {}

  FieldValue(const char *value):
    str_val{value ? value : ""}, type{STRING} {}

  FieldValue(int value):
    int_val{value}, type{INT} {}

  FieldValue(uint64_t value):
    int64_val{value}, type{INT64} {}

  FieldValue(bool value):
    bool_val{value}, type{BOOL} {}

  FieldValue(double value):
    real_val{value}, type{REAL} {}

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
  // Clear the row
  void clear()
  { fields.clear(); }

  //------------------------------------------------------------------------
  // Add name/value pair (from a FieldValue)
  Row& add(const string& fieldname, const FieldValue& value)
  { fields[fieldname] = FieldValue{value}; return *this; }

  //------------------------------------------------------------------------
  // Handy operator to add null values to a row, for select()
  Row& operator<<(const string& fieldname)
  { fields[fieldname] = FieldValue{}; return *this; }

  //------------------------------------------------------------------------
  // Add name/value pair
  void add(const string& fieldname, const string& value)
  { fields[fieldname] = FieldValue{value}; }

  //------------------------------------------------------------------------
  // Add char * value to row
  void add(const string& fieldname, const char *value)
  { fields[fieldname] = FieldValue{value}; }

  //------------------------------------------------------------------------
  // Add name/value pair (NULL if empty)
  void add_or_null(const string& fieldname, const string& value)
  {
    if (value.size())
      fields[fieldname] = FieldValue{value};
    else
      fields[fieldname] = FieldValue{};
  }

  //------------------------------------------------------------------------
  // Add a NULL entry
  void add_null(const string& fieldname)
  { fields[fieldname] = FieldValue{}; }

  //------------------------------------------------------------------------
  // Add integer value to row
  void add(const string& fieldname, int value)
  { fields[fieldname] = FieldValue{value}; }

  //------------------------------------------------------------------------
  // Add integer value to row (NULL if zero)
  void add_or_null(const string& fieldname, int value)
  {
    if (value)
      fields[fieldname] = FieldValue{value};
    else
      fields[fieldname] = FieldValue{};
  }

  //------------------------------------------------------------------------
  // Add a 64-bit integer value to row
  void add_int64(const string& fieldname, uint64_t value)
  { fields[fieldname] = FieldValue{value}; }

  //------------------------------------------------------------------------
  // Add a 64-bit integer value to row (NULL if zero)
  void add_int64_or_null(const string& fieldname, uint64_t value)
  {
    if (value)
      fields[fieldname] = FieldValue{value};
    else
      fields[fieldname] = FieldValue{};
  }

  //------------------------------------------------------------------------
  // Add boolean value to row
  void add(const string& fieldname, bool value)
  { fields[fieldname] = FieldValue{value}; }

  //------------------------------------------------------------------------
  // Add real value to row
  void add(const string& fieldname, double value)
  { fields[fieldname] = FieldValue{value}; }

  //------------------------------------------------------------------------
  // Add real value to row (NULL if zero)
  void add_or_null(const string& fieldname, double value)
  {
    if (value)
      fields[fieldname] = FieldValue{value};
    else
      fields[fieldname] = FieldValue{};
  }

  //------------------------------------------------------------------------
  // Add a timestamp to a row, NULL if invalid
  void add_time_or_null(const string& fieldname, const Time::Stamp& value)
  {
    if (value.valid())
      fields[fieldname] = FieldValue{value.sql()};
    else
      fields[fieldname] = FieldValue{};
  }

  //------------------------------------------------------------------------
  // Add a date to a row, NULL if invalid
  void add_date_or_null(const string& fieldname,
                        const Time::DateStamp& value)
  {
    if (value.valid())
      fields[fieldname] = FieldValue{value.sql()};
    else
      fields[fieldname] = FieldValue{};
  }

  //------------------------------------------------------------------------
  // Finds whether the row contains a value for the given fieldname
  bool has(const string& fieldname) const
  { return fields.find(fieldname) != fields.end(); }

  //------------------------------------------------------------------------
  // Get value of field of given name, or default if not found
  string get(const string& fieldname, const string& def="") const;

  //------------------------------------------------------------------------
  // Handy [] operator
  // e.g. foo = row["id"];
  string operator[](const string& fieldname) const
  { return get(fieldname); }

  //------------------------------------------------------------------------
  // Get integer value of field of given name, or default if not found
  int get_int(const string& fieldname, int def=0) const;

  //------------------------------------------------------------------------
  // Get 64-bit value of field of given name, or default if not found
  uint64_t get_int64(const string& fieldname, uint64_t def=0) const;

  //------------------------------------------------------------------------
  // Get boolean value of field of given name, or default if not found
  bool get_bool(const string& fieldname, bool def=false) const;

  //------------------------------------------------------------------------
  // Get real value of field of given name, or default if not found
  double get_real(const string& fieldname, double def=0) const;

  //------------------------------------------------------------------------
  // Get string with field names in order, separated by commas and spaces
  string get_fields() const;

  //--------------------------------------------------------------------------
  // Get fields that are *not* in a suppressed fields row
  string get_fields_not_in(const Row& suppressed_fields) const;

  //--------------------------------------------------------------------------
  // Get string with field values in order, separated by commas and spaces,
  // with assigment back to VALUES(name) - e.g. x=VALUES(x), y=VALUES(y)
  // (e.g. for INSERT .. ON DUPLICATE KEY UPDATE)
  string get_fields_set_to_own_values() const;

  //------------------------------------------------------------------------
  // Get string with field values in order, separated by commas and spaces,
  // each escaped and delimited with single quotes (e.g. for INSERT)
  string get_escaped_values() const;

  //------------------------------------------------------------------------
  // Get string with field names and values in order with '=',
  // separated by commas and spaces, values delimited with single quotes
  // (e.g. for UPDATE)
  string get_escaped_assignments() const;

  //--------------------------------------------------------------------------
  // Get string with field names and values in order with '=',
  // separated by commas and spaces, values delimited with single quotes
  // limited by another row (e.g. for ON CONFLICT ... DO UPDATE)
  string get_escaped_assignments_limited_by(const Row& limit) const;

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
  // Get number of rows in result set
  virtual int count() = 0;

  //------------------------------------------------------------------------
  // Get next row from result set
  // Whether another was found - if so, clears and writes into row
  virtual bool fetch(Row& row) = 0;

  //------------------------------------------------------------------------
  // Get first value of next row from result set
  // Value is unescaped
  // Whether another was found - if so, writes into value
  virtual bool fetch(string& value) = 0;

  //------------------------------------------------------------------------
  // Virtual destructor
  virtual ~ResultSet() {}
};

//==========================================================================
// Database result auto-ptr wrapper
// Allows inheritance of ResultSet while preserving single abstract interface
class Result
{
private:
  unique_ptr<ResultSet> rset;

public:
  //------------------------------------------------------------------------
  // Constructors
  Result() = default;                // Invalid result
  Result(ResultSet *r): rset{r} {}   // Valid result

  //------------------------------------------------------------------------
  // Handy bool cast to check for (in)validity
  explicit operator bool() const { return rset.get(); }

  //------------------------------------------------------------------------
  // Get number of rows in result set
  int count() { return rset ? rset->count() : 0; }

  //------------------------------------------------------------------------
  // Get next row from result set
  // Whether another was found - if so, clears writes into row
  bool fetch(Row& row) { return rset && rset->fetch(row); }

  //------------------------------------------------------------------------
  // Get first value of next row from result set
  // Whether another was found - if so, writes into value
  bool fetch(string& value) { return rset && rset->fetch(value); }
};

//==========================================================================
// Abstract Prepared Statement
class PreparedStatement: public ResultSet
{
public:
  //------------------------------------------------------------------------
  // Bind a parameter (bool)
  virtual bool bind(int index, bool value) = 0;

  //------------------------------------------------------------------------
  // Bind a parameter (integer)
  virtual bool bind(int index, int64_t value) = 0;

  //------------------------------------------------------------------------
  // Bind a parameter (unsigned integer)
  virtual bool bind(int index, uint64_t value) = 0;

  //------------------------------------------------------------------------
  // Bind a parameter (unsigned integer)
  virtual bool bind(int index, unsigned value) = 0;

  //------------------------------------------------------------------------
  // Bind a parameter (real)
  virtual bool bind(int index, double value) = 0;

  //------------------------------------------------------------------------
  // Bind a parameter (text)
  virtual bool bind(int index, const string& value) = 0;

  //------------------------------------------------------------------------
  // Bind a parameter (null)
  virtual bool bind(int index) = 0;

  //------------------------------------------------------------------------
  // Reset statement
  virtual void reset() = 0;

  //------------------------------------------------------------------------
  // Execute statement
  virtual bool execute() = 0;

  //------------------------------------------------------------------------
  // Move to next row
  virtual bool next() = 0;

  //------------------------------------------------------------------------
  // Fetch field as string
  virtual string get_string(int col) = 0;

  //------------------------------------------------------------------------
  // Fetch field as int
  virtual uint64_t get_int(int col) = 0;

  //------------------------------------------------------------------------
  // Fetch field as double
  virtual double get_real(int col) = 0;

  //------------------------------------------------------------------------
  // Fetch field as Time::Stamp
  virtual Time::Stamp get_time(int col) = 0;

  //------------------------------------------------------------------------
  // Is valid?
  virtual explicit operator bool() const = 0;
};

//==========================================================================
// Prepared Statement Wrapper
class Statement: public PreparedStatement
{
private:
  unique_ptr<PreparedStatement> statement;

public:
  //------------------------------------------------------------------------
  // Constructor
  Statement() = default;
  Statement(PreparedStatement *_statement):
    statement{_statement}
  {}

  //------------------------------------------------------------------------
  // Is valid
  explicit operator bool() const override
  {
    return !!*statement;
  }

  //------------------------------------------------------------------------
  // Bind a parameter (bool)
  bool bind(int index, bool value) override
  {
    return statement && statement->bind(index, value);
  }

  //------------------------------------------------------------------------
  // Bind a parameter (integer)
  bool bind(int index, int64_t value) override
  {
    return statement && statement->bind(index, value);
  }

  //------------------------------------------------------------------------
  // Bind a parameter (unsigned integer)
  bool bind(int index, uint64_t value) override
  {
    return statement && statement->bind(index, value);
  }

  //------------------------------------------------------------------------
  // Bind a parameter (unsigned integer)
  bool bind(int index, unsigned value) override
  {
    return statement && statement->bind(index, value);
  }

  //------------------------------------------------------------------------
  // Bind a parameter (real)
  bool bind(int index, double value) override
  {
    return statement && statement->bind(index, value);
  }

  //------------------------------------------------------------------------
  // Bind a parameter (text)
  bool bind(int index, const string& value) override
  {
    return statement && statement->bind(index, value);
  }

  //------------------------------------------------------------------------
  // Bind a parameter (null)
  bool bind(int index) override
  {
    return statement && statement->bind(index);
  }

  //------------------------------------------------------------------------
  // Reset statement
  void reset() override
  {
    if (statement) statement->reset();
  }

  //------------------------------------------------------------------------
  // Execute statement
  bool execute() override
  {
    return statement && statement->execute();
  }

  //------------------------------------------------------------------------
  // Get number of rows in result set
  int count() override
  {
    return statement && statement->count();
  }

  //------------------------------------------------------------------------
  // Get next row from result set
  // Whether another was found - if so, clears writes into row
  bool fetch(Row& row) override
  {
    return statement && statement->fetch(row);
  }

  //------------------------------------------------------------------------
  // Get first value of next row from result set
  // Whether another was found - if so, writes into value
  bool fetch(string& value) override
  {
    return statement && statement->fetch(value);
  }

  //------------------------------------------------------------------------
  // Move to next row
  bool next() override
  {
    return statement && statement->next();
  }

  //------------------------------------------------------------------------
  // Fetch field as string
  string get_string(int col) override
  {
    return statement ? statement->get_string(col) : "";
  }

  //------------------------------------------------------------------------
  // Fetch field as int
  uint64_t get_int(int col) override
  {
    return statement ? statement->get_int(col) : 0;
  }

  //------------------------------------------------------------------------
  // Fetch field as double
  double get_real(int col) override
  {
    return statement ? statement->get_real(col) : 0;
  }

  //------------------------------------------------------------------------
  // Fetch field as Time::Stamp
  Time::Stamp get_time(int col) override
  {
    return statement ? statement->get_time(col) : Time::Stamp{};
  }
};

//==========================================================================
// AutoStatement
// resets a statement on destruction
class AutoStatement: public PreparedStatement
{
private:
  Statement *statement = nullptr;

public:
  //------------------------------------------------------------------------
  // Constructor
  AutoStatement(Statement *_statement):
    statement{_statement}
  {}

  //------------------------------------------------------------------------
  // Copy assignment operator
  AutoStatement& operator=(const AutoStatement& b)
  {
    if (statement) statement->reset();
    statement = b.statement;
    return *this;
  }

  //------------------------------------------------------------------------
  // Is valid
  explicit operator bool() const override
  {
    return statement && *statement;
  }

  //------------------------------------------------------------------------
  // Bind a parameter (bool)
  bool bind(int index, bool value) override
  {
    return statement && statement->bind(index, value);
  }

  //------------------------------------------------------------------------
  // Bind a parameter (integer)
  bool bind(int index, int64_t value) override
  {
    return statement && statement->bind(index, value);
  }

  //------------------------------------------------------------------------
  // Bind a parameter (unsigned integer)
  bool bind(int index, uint64_t value) override
  {
    return statement && statement->bind(index, value);
  }

  //------------------------------------------------------------------------
  // Bind a parameter (unsigned integer)
  bool bind(int index, unsigned value) override
  {
    return statement && statement->bind(index, value);
  }

  //------------------------------------------------------------------------
  // Bind a parameter (real)
  bool bind(int index, double value) override
  {
    return statement && statement->bind(index, value);
  }

  //------------------------------------------------------------------------
  // Bind a parameter (text)
  bool bind(int index, const string& value) override
  {
    return statement && statement->bind(index, value);
  }

  //------------------------------------------------------------------------
  // Bind a parameter (null)
  bool bind(int index) override
  {
    return statement && statement->bind(index);
  }

  //------------------------------------------------------------------------
  // Reset statement
  void reset() override
  {
    if (statement) statement->reset();
  }

  //------------------------------------------------------------------------
  // Execute statement
  bool execute() override
  {
    return statement && statement->execute();
  }

  //------------------------------------------------------------------------
  // Get number of rows in result set
  int count() override
  {
    return statement && statement->count();
  }

  //------------------------------------------------------------------------
  // Get next row from result set
  // Whether another was found - if so, clears writes into row
  bool fetch(Row& row) override
  {
    return statement && statement->fetch(row);
  }

  //------------------------------------------------------------------------
  // Get first value of next row from result set
  // Whether another was found - if so, writes into value
  bool fetch(string& value) override
  {
    return statement && statement->fetch(value);
  }

  //------------------------------------------------------------------------
  // Move to next row
  bool next() override
  {
    return statement && statement->next();
  }

  //------------------------------------------------------------------------
  // Fetch field as string
  string get_string(int col) override
  {
    return statement ? statement->get_string(col) : "";
  }

  //------------------------------------------------------------------------
  // Fetch field as int
  uint64_t get_int(int col) override
  {
    return statement ? statement->get_int(col) : 0;
  }

  //------------------------------------------------------------------------
  // Fetch field as double
  double get_real(int col) override
  {
    return statement ? statement->get_real(col) : 0;
  }

  //------------------------------------------------------------------------
  // Fetch field as Time::Stamp
  Time::Stamp get_time(int col) override
  {
    return statement ? statement->get_time(col) : Time::Stamp{};
  }

  //------------------------------------------------------------------------
  // Destructor
  ~AutoStatement()
  {
    if (statement) statement->reset();
  }
};

//==========================================================================
// Database connection (abstract)
class Connection
{
protected:
  map<string, Statement> prepared_statements;

public:
  //========================================================================
  // Virtual functions implemented by driver subclass

  //------------------------------------------------------------------------
  // Check if connection is really OK
  virtual explicit operator bool() = 0;

  //------------------------------------------------------------------------
  // Execute a command, not expecting any result (e.g. INSERT, UPDATE, DELETE)
  // Returns whether successful
  virtual bool exec(const string& sql) = 0;

  //------------------------------------------------------------------------
  // Execute a query and get result (e.g. SELECT)
  // Returns result - check this for validity
  virtual Result query(const string& sql) = 0;

  //------------------------------------------------------------------------
  // Prepare a statement
  // Returns result - check this for validity
  virtual Statement prepare(const string& sql) = 0;

  //------------------------------------------------------------------------
  // Gets the last insert id
  virtual uint64_t get_last_insert_id() = 0;

  //------------------------------------------------------------------------
  // Begin a transaction
  virtual bool transaction_begin()
  {
    return exec("begin");
  }

  //------------------------------------------------------------------------
  // Begin a transaction with immediate lock
  virtual bool transaction_begin_immediate()
  {
    return exec("begin immediate");
  }

  //------------------------------------------------------------------------
  // Commit a transaction
  virtual bool transaction_commit()
  {
    return exec("commit");
  }

  //------------------------------------------------------------------------
  // Roll back a transaction
  virtual bool transaction_rollback()
  {
    return exec("rollback");
  }

  //------------------------------------------------------------------------
  // Expression to get current datetime in UTC
  virtual string utc_timestamp() = 0;

  //------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Connection() {}

  //========================================================================
  // Helper functions implemented in connection.cc

  //------------------------------------------------------------------------
  // Create a held prepared statement
  bool prepare_statement(const string& id, const string& sql);

  //------------------------------------------------------------------------
  // Get a held prepared statement for use
  // Note: user is responsible for ensuring thread safety
  AutoStatement get_statement(const string& id);

  //------------------------------------------------------------------------
  // Execute a query and get first (only) row
  // Returns whether successful - row is cleared and filled in if so
  bool query(const string& sql, Row& row);

  //------------------------------------------------------------------------
  // Execute a query and get single (only) value from first (only) row
  // Returns whether successful - value is filled in if so
  bool query(const string& sql, string& value);

  //------------------------------------------------------------------------
  // Execute a query and get single (only) value from first (only) row
  // Returns value or default if not found
  string query_string(const string& sql, const string& def="");

  //------------------------------------------------------------------------
  // Execute a query and get single (only) integer value from first (only) row
  // Returns value or default if not found
  int query_int(const string& sql, int def=0);

  //------------------------------------------------------------------------
  // Execute a query and get single (only) 64-bit integer value from first
  // (only) row
  // Returns value or default if not found
  uint64_t query_int64(const string& sql, uint64_t def=0);

  //------------------------------------------------------------------------
  // Execute a query and get single (only) integer value from first (only) row
  // Returns value or default if not found
  bool query_bool(const string& sql, bool def=false);

  //------------------------------------------------------------------------
  // Do an INSERT and retrieve the last inserted (max) automatic ID
  // Returns ID, or 0 if failed
  // Fetches max(<id_field>) inside a transaction, unless id_field is ""
  // If id_field is "", does a normal insert and returns 1
  // Set in_transaction only if you're already doing a transaction; by
  // default this function may create its own
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

  //--------------------------------------------------------------------------
  // Do an INSERT or UPDATE if it already exists (violates unique key)
  // Uses INSERT ... ON DUPLICATE KEY UPDATE
  // Each field in the row is inserted by name
  // update_row gives the list of fields from row which are updated (not
  // part of the unique key)
  // Note: All fields are escaped on insertion
  // Returns whether successful
  virtual bool insert_or_update(const string& table, Row& row, Row& update_row);

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
  // If distinct is set, does SELECT DISTINCT
  // Returns query result as query()
  Result select(const string& table, const Row& row, const string& where="",
                bool distinct = false);

  //------------------------------------------------------------------------
  // Do a SELECT for all fields in the given row in the given table
  // matching the list of values in where_row
  // If distinct is set, does SELECT DISTINCT
  // Returns query result as query()
  Result select(const string& table, const Row& row, const Row& where_row,
                bool distinct = false);

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
protected:
  map<string, string> prepared_statements;

  //------------------------------------------------------------------------
  // Create a new connection
  virtual Connection *create_connection() = 0;

public:
  //------------------------------------------------------------------------
  // Constructors
  ConnectionFactory() = default;
  ConnectionFactory(const map<string, string>& _prepared_statements):
    prepared_statements{_prepared_statements}
  {}

  //------------------------------------------------------------------------
  // create a new connection
  Connection *create();

  //------------------------------------------------------------------------
  // Virtual destructor
  virtual ~ConnectionFactory() {}
};

//==========================================================================
// Database connection pool - maintains list of database connections which
// can be claimed and released
class ConnectionPool: MT::Thread
{
  ConnectionFactory& factory;
  unsigned min_connections;        // Number started on creation
  unsigned max_connections;        // Maximum ever created
  Time::Duration max_inactivity;

  // Current connections
  mutable MT::Mutex mutex;         // On connection lists
  list<Connection *> connections;  // All connections
  list<Connection *> available;    // Connections available for use

  // Pending claim requests
  struct PendingRequest
  {
    Time::Stamp started=Time::Stamp::now();  // When started
    Connection *connection=nullptr;          // Filled in when available
    MT::Condition available;                 // Signal availability
  };
  list<shared_ptr<PendingRequest> > pending_requests;
  Time::Duration claim_timeout{5.0};

  // Background thread and timestamps
  map<Connection *, Time::Stamp> last_used;
  Time::Duration reap_interval{1.0};

  // Internals
  void fill_to_minimum();

  //------------------------------------------------------------------------
  // Run background timeout loop (called from internal thread)
  void run() override;

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
  // Set the reap interval
  void set_reap_interval(const Time::Duration &i) { reap_interval = i; }

  //------------------------------------------------------------------------
  // Set the claim timeout
  void set_claim_timeout(const Time::Duration &t) { claim_timeout = t; }

  //------------------------------------------------------------------------
  // Get number of connections created
  unsigned num_connections() const
  { MT::Lock lock(mutex); return connections.size(); }

  //------------------------------------------------------------------------
  // Get number of connections available
  unsigned num_available() const
  { MT::Lock lock(mutex); return available.size(); }

  //------------------------------------------------------------------------
  // Get number of connections in use
  unsigned num_in_use() const
  { MT::Lock lock(mutex); return connections.size()-available.size(); }

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
  explicit operator bool() const { return conn; }

  //------------------------------------------------------------------------
  // Stub functions for every connection operation - see above
  bool exec(const string& sql) { return conn?conn->exec(sql):false; }

  Result query(const string& sql) { return conn?conn->query(sql):Result(); }

  AutoStatement get_statement(const string& id)
  {
    return conn ? conn->get_statement(id) : nullptr;
  }

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

  uint64_t get_last_insert_id()
  { return conn?conn->get_last_insert_id():0; }

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

  bool insert_or_update(const string& table, Row& row, Row& update_row)
  { return conn?conn->insert_or_update(table, row, update_row):false; }

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

  string utc_timestamp() { return conn?conn->utc_timestamp():""; }

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
#ifdef DEBUG
  chrono::high_resolution_clock::time_point
    start = chrono::high_resolution_clock::now();
  chrono::high_resolution_clock::duration begun_at;
#endif

public:
  //------------------------------------------------------------------------
  // Constructor from plain connection
  Transaction(Connection& _conn, bool immediate = false);

  //------------------------------------------------------------------------
  // Constructor from AutoConnection
  Transaction(AutoConnection& _conn, bool immediate = false);

  //------------------------------------------------------------------------
  // Commit - returns whether commit command ran OK
  bool commit();

  //------------------------------------------------------------------------
  // Destructor
  ~Transaction();
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_DB_H
