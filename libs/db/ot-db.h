//==========================================================================
// ObTools::DB: ot-db.h
//
// Public definitions for ObTools::DB Library
// General C++ database wrapper
// 
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_DB_H
#define __OBTOOLS_DB_H

#include <map>
#include <iostream>
#include <stdint.h>

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
// Database row (same for all drivers)
struct Row
{
  map<string, string> fields;

  //------------------------------------------------------------------------
  //Clear the row
  void clear() 
  { fields.clear(); }

  //------------------------------------------------------------------------
  //Add name/value pair
  void add(const string& fieldname, const string& value)
  { fields[fieldname] = value; }

  //------------------------------------------------------------------------
  //Add name/value pair, unescaping value (for use by drivers only)
  void add_unescaped(const string& fieldname, const string& value)
  { fields[fieldname] = unescape(value); }

  //------------------------------------------------------------------------
  // Add integer value to row
  void add(string fieldname, int value);

  //------------------------------------------------------------------------
  // Add boolean value to row
  void add(string fieldname, bool value);

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
  // Get string with field names in order, separated by commas and spaces
  string get_fields() const; 

  //------------------------------------------------------------------------
  // Get string with field values in order, separated by commas and spaces,
  // each escaped and delimited with single quotes (e.g. for INSERT)
  string get_escaped_values() const;

  //==========================================================================
  // Static helper functions

  //------------------------------------------------------------------------
  // Escape a string, doubling single quotes and backslashes
  static string escape(const string& s);

  //------------------------------------------------------------------------
  // Unescape a string, singling double quotes and backslashes
  static string unescape(const string& s);
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
  //Execute a query and get single (only) integer value from first (only) row
  //Returns value or default if not found
  bool query_bool(const string& sql, bool def=false);

  //------------------------------------------------------------------------
  //Do an INSERT and retrieve the last inserted (max) automatic ID
  //Returns ID, or 0 if failed
  //Note:  You can do an ordinary exec() for INSERT if you don't care about
  //the auto-generated ID used - it's probably faster
  //Set in_transaction only if you're already doing a transaction; by
  //default this function may create its own
  int insert(const string& sql, 
	     const string& table, const string& id_field="id",
	     bool in_transaction=false);

  //------------------------------------------------------------------------
  // Do an INSERT and retrieve the last inserted serial ID, from row data
  // Each field in the row is inserted by name
  // Returns ID, or 0 if failed
  int insert(Row& row, const string& table, const string& id_field="id",
	     bool in_transaction=false);
};

//==========================================================================
// Transaction - provide transaction on given connection while it exists,
// rolls back if killed without commit
class Transaction
{
private:
  Connection& conn;
  bool committed;

public:
  //------------------------------------------------------------------------
  //Constructor 
  Transaction(Connection& _conn);

  //------------------------------------------------------------------------
  //Commit - returns whether commit command ran OK
  bool commit();

  //------------------------------------------------------------------------
  //Destructor
  ~Transaction();
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

#if !defined(_SINGLE)
// Connection pooling makes no sense in single threaded mode - you should
// hold a single global connection open for the lifetime of the application

//==========================================================================
// Database connection pool - maintains list of database connections which
// can be claimed and released
class ConnectionPool
{
  ConnectionFactory& factory;
  unsigned min_connections;        // Number started on creation
  unsigned max_connections;        // Maximum ever created

  // Current connections
  MT::Mutex mutex;                 // On connection lists
  list<Connection *> connections;  // All connections
  list<Connection *> available;    // Connections available for use

public:
  //------------------------------------------------------------------------
  // Constructor
  ConnectionPool(ConnectionFactory& _factory, unsigned _min, unsigned _max);

  //------------------------------------------------------------------------
  // Claim a connection
  // Returns connection, or 0 if one could not be created or all are active
  Connection *claim();

  //------------------------------------------------------------------------
  // Release a connection after use
  void release(Connection *conn);

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

  bool query_bool(const string& sql, bool def=false)
  { return conn?conn->query_bool(sql, def):def; }

  int insert(const string& sql, 
	     const string& table, const string& id_field="id",
	     bool in_transaction=false)
  { return conn?conn->insert(sql, table, id_field, in_transaction):0; }

  int insert(Row& row, const string& table, const string& id_field="id",
	     bool in_transaction=false)
  { return conn?conn->insert(row, table, id_field, in_transaction):0; }

  static string escape(const string& s) { return Row::escape(s); }
  static string unescape(const string& s) { return Row::unescape(s); }

  //------------------------------------------------------------------------
  // Destructor
  ~AutoConnection() { if (conn) pool.release(conn); }
};

#endif // !_SINGLE

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_DB_H



