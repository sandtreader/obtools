//==========================================================================
// ObTools::DB: ot-db-pgsql.h
//
// Definition of Postgres-specific database driver
// 
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_DB_PGSQL_H
#define __OBTOOLS_DB_PGSQL_H

#include "ot-db.h"

namespace ObTools { namespace DB { namespace PG {  

//Make our lives easier without polluting anyone else
using namespace std;

// Note we use void *'s here to avoid forcing our uses to include libpq.h

//==========================================================================
//Postgres result class
class ResultSet: public DB::ResultSet
{ 
private:
  void *pgres;     // PGresult structure
  int row_cursor;  // Row cursor

public:
  //------------------------------------------------------------------------
  //Constructor
  ResultSet(void *res): pgres(res), row_cursor(0) {}

  //------------------------------------------------------------------------
  //Get number of rows in result set
  int count();

  //------------------------------------------------------------------------
  //Get next row from result set
  //Whether another was found - if so, writes into row
  bool fetch(Row& row);

  //------------------------------------------------------------------------
  //Get first value of next row from result set
  //Whether another was found - if so, writes into value
  bool fetch(string& value);

  //------------------------------------------------------------------------
  //Destructor
  ~ResultSet();
};

//==========================================================================
//Postgres connection class
class Connection: public DB::Connection
{
private:
  void *pgconn;   // PGconn structure
  int last_oid;   // Last OID to be inserted

public:
  //------------------------------------------------------------------------
  //Constructor - takes Postgres connection string
  //e.g. "host=localhost dbname=foo user=prc password=secret"
  Connection(const string& conninfo);

  //------------------------------------------------------------------------
  //Execute a command, not expecting any result (e.g. INSERT, UPDATE, DELETE)
  //Returns whether successful
  bool exec(const string& sql);

  //------------------------------------------------------------------------
  //Execute a query and get result (e.g. SELECT)
  //Returns result - check this for validity
  Result query(const string& sql);

  //------------------------------------------------------------------------
  //Get integer ID of last INSERT
  int inserted_id();

  //------------------------------------------------------------------------
  //Destructor
  ~Connection();
};

//==========================================================================
}}} //namespaces
#endif // !__OBTOOLS_DB_PGSQL_H



