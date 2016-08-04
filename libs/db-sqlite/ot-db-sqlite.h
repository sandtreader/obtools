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
  // Bind a parameter (integer)
  bool bind(int index, int64_t value) override;

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
  // Is valid?
  operator bool() const override
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
  Connection(const string& filename);

  //------------------------------------------------------------------------
  // Check if connection is really OK
  operator bool() override;

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
};

//==========================================================================
// SQLite connection factory
class ConnectionFactory: public DB::ConnectionFactory
{
  // Connection details
  string file;

public:
  //------------------------------------------------------------------------
  // Constructor
  ConnectionFactory(const string& _file):
    file(_file)
  {}

  //------------------------------------------------------------------------
  // Interface to create a new connection
  DB::Connection *create()
  {
    return new Connection(file);
  }
};

//==========================================================================
}}} //namespaces
#endif // !__OBTOOLS_DB_SQLITE_H
