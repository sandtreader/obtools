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

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// SQLite result class
class ResultSet: public DB::ResultSet
{
private:
  unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt;
  int num_fields;
  vector<string> field_names;

public:
  //------------------------------------------------------------------------
  // Constructor
  ResultSet(sqlite3_stmt *stmt);

  //------------------------------------------------------------------------
  // Get number of rows in result set
  int count();

  //------------------------------------------------------------------------
  // Get next row from result set
  // Whether another was found - if so, writes into row
  bool fetch(Row& row);

  //------------------------------------------------------------------------
  // Get first value of next row from result set
  // Value is unescaped
  // Whether another was found - if so, writes into value
  bool fetch(string& value);
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
  bool ok();

  //------------------------------------------------------------------------
  // Execute a command, not expecting any result (e.g. INSERT, UPDATE, DELETE)
  // Returns whether successful
  bool exec(const string& sql);

  //------------------------------------------------------------------------
  // Execute a query and get result (e.g. SELECT)
  // Returns result - check this for validity
  Result query(const string& sql);
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
