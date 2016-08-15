//==========================================================================
// ObTools::DB: ot-db-pgsql.h
//
// Definition of Postgres-specific database driver
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_DB_PGSQL_H
#define __OBTOOLS_DB_PGSQL_H

#include "ot-db.h"
#include "ot-log.h"
#include <libpq-fe.h>

namespace ObTools { namespace DB { namespace PG {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Postgres result class
class ResultSet: public DB::ResultSet
{
private:
  PGresult *res = nullptr;  // PGresult structure
  int row_cursor = 0;       // Row cursor

public:
  //------------------------------------------------------------------------
  // Constructor
  ResultSet(PGresult *_res): res{_res} {}

  //------------------------------------------------------------------------
  // Get number of rows in result set
  int count() override;

  //------------------------------------------------------------------------
  // Get next row from result set
  // Whether another was found - if so, writes into row
  bool fetch(Row& row) override;

  //------------------------------------------------------------------------
  // Get first value of next row from result set
  // Value is unescaped
  // Whether another was found - if so, writes into value
  bool fetch(string& value) override;

  //------------------------------------------------------------------------
  // Destructor
  ~ResultSet();
};

//==========================================================================
// Postgres connection class
class Connection: public DB::Connection
{
private:
  PGconn *conn = nullptr; // PGconn structure
  Log::Streams log; // Private (therefore per-thread, assuming connections
                    // are not shared) log streams

public:
  //------------------------------------------------------------------------
  // Constructor - takes Postgres connection string
  // e.g. "host=localhost dbname=foo user=prc password=secret"
  Connection(const string& conninfo);

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
  Statement prepare(const string& sql) override
  {
    throw runtime_error("Prepare not implemented for PGSQL");
  }

  //------------------------------------------------------------------------
  // Gets the last insert id
  uint64_t get_last_insert_id() override
  {
    throw runtime_error("Last insert id not implemented for PGSQL");
  }

  //------------------------------------------------------------------------
  // Destructor
  ~Connection();
};

//==========================================================================
// PostgreSQL connection factory
class ConnectionFactory: public DB::ConnectionFactory
{
  // Connection details
  string conninfo;

  //------------------------------------------------------------------------
  // Interface to create a new connection
  DB::Connection *create_connection() override
  { return new PG::Connection(conninfo); }

public:
  //------------------------------------------------------------------------
  // Constructors
  ConnectionFactory(const string& _conninfo):  conninfo(_conninfo) {}
  ConnectionFactory(const string& _conninfo,
                    const map<string, string>& statements):
    DB::ConnectionFactory{statements}, conninfo(_conninfo) {}
};

//==========================================================================
}}} // namespaces
#endif // !__OBTOOLS_DB_PGSQL_H
