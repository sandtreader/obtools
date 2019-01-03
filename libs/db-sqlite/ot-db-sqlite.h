//==========================================================================
// ObTools::DB: ot-db-sqlite.h
//
// Definition of SQLite-specific database driver
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_DB_SQLITE_H
#define __OBTOOLS_DB_SQLITE_H

#include "ot-db.h"
#include "ot-log.h"
#include <sqlite3.h>

namespace ObTools { namespace DB { namespace SQLite {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Prepared Statement
// Note: Writes will be locked out until this is freed!!!
class PreparedStatement: public DB::PreparedStatement
{
private:
  unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt;
  int num_fields;
  vector<string> field_names;

public:
  //------------------------------------------------------------------------
  // Constructor
  PreparedStatement(sqlite3_stmt *stmt);

  //------------------------------------------------------------------------
  // Bind a parameter (bool)
  bool bind(int index, bool value) override
  {
    return bind(index, static_cast<int64_t>(value));
  }

  //------------------------------------------------------------------------
  // Bind a parameter (integer)
  bool bind(int index, int64_t value) override;

  //------------------------------------------------------------------------
  // Bind a parameter (unsigned integer)
  bool bind(int index, uint64_t value) override
  {
    return bind(index, static_cast<int64_t>(value));
  }

  //------------------------------------------------------------------------
  // Bind a parameter (unsigned integer)
  bool bind(int index, unsigned value) override
  {
    return bind(index, static_cast<int64_t>(value));
  }

  //------------------------------------------------------------------------
  // Bind a parameter (real)
  bool bind(int index, double value) override;

  //------------------------------------------------------------------------
  // Bind a parameter (text)
  bool bind(int index, const string& value) override;

  //------------------------------------------------------------------------
  // Bind a parameter (null)
  bool bind(int index) override;

  //------------------------------------------------------------------------
  // Reset statement
  void reset() override;

  //------------------------------------------------------------------------
  // Execute statement
  bool execute() override;

  //------------------------------------------------------------------------
  // Get row count
  int count() override;

  //------------------------------------------------------------------------
  // Get next row from result set
  bool fetch(Row& row) override;

  //------------------------------------------------------------------------
  // Get first value of next row from result set
  bool fetch(string& value) override;

  //------------------------------------------------------------------------
  // Move to next row
  bool next() override;

  //------------------------------------------------------------------------
  // Fetch field as string
  string get_string(int col) override;

  //------------------------------------------------------------------------
  // Fetch field as int
  uint64_t get_int(int col) override;

  //------------------------------------------------------------------------
  // Fetch field as double
  double get_real(int col) override;

  //------------------------------------------------------------------------
  // Fetch field as Time::Stamp
  Time::Stamp get_time(int col) override;

  //------------------------------------------------------------------------
  // Is valid?
  explicit operator bool() const override
  {
    return stmt.get();
  }
};

//==========================================================================
// SQLite result class
// Note: in SQLite, queries are done using prepared statements
class ResultSet: public PreparedStatement
{
public:
  //------------------------------------------------------------------------
  // Constructor
  ResultSet(sqlite3_stmt *stmt):
    PreparedStatement{stmt}
  {}
};

//==========================================================================
// SQLite connection class
class Connection: public DB::Connection
{
private:
  unique_ptr<sqlite3, decltype(&sqlite3_close)> conn;

public:
  //------------------------------------------------------------------------
  // Constructor
  Connection(const string& filename, const Time::Duration& timeout);

  //------------------------------------------------------------------------
  // Check if connection is really OK
  explicit operator bool() override;

  //------------------------------------------------------------------------
  // Execute a command, not expecting any result (e.g. INSERT, UPDATE, DELETE)
  // Returns whether successful
  bool exec(const string& sql) override;

  //------------------------------------------------------------------------
  // Execute a query and get result (e.g. SELECT)
  // Returns result - check this for validity
  Result query(const string& sql) override;

  //------------------------------------------------------------------------
  // Prepare a statement
  // Returns result - check this for validity
  Statement prepare(const string& sql) override;

  //------------------------------------------------------------------------
  // Gets the last insert id
  uint64_t get_last_insert_id() override;

  //------------------------------------------------------------------------
  // Destructor
  ~Connection()
  {
    // SQLite needs all prepared statements to be finalized before closing
    // the connection
    prepared_statements.clear();
  }
};

//==========================================================================
// SQLite connection factory
class ConnectionFactory: public DB::ConnectionFactory
{
  // Connection details
  string file;
  Time::Duration timeout;

  //------------------------------------------------------------------------
  // Interface to create a new connection
  DB::Connection *create_connection() override
  {
    return new Connection{file, timeout};
  }

public:
  //------------------------------------------------------------------------
  // Constructors
  ConnectionFactory(const string& _file, const Time::Duration& _timeout):
    file{_file}, timeout{_timeout}
  {}
  ConnectionFactory(const string& _file,
                    const Time::Duration& _timeout,
                    const map<string, string>& statements):
    DB::ConnectionFactory{statements}, file{_file}, timeout{_timeout}
  {}
};

//==========================================================================
}}} //namespaces
#endif // !__OBTOOLS_DB_SQLITE_H
