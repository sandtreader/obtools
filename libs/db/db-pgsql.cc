//==========================================================================
// ObTools::DB: db-pgsql.cc
//
// PostgresQL Database wrapper implementation
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#include "ot-db-pgsql.h"
#include "ot-log.h"
#include "libpq-fe.h"

namespace ObTools { namespace DB { namespace PG {

//==========================================================================
//Postgres result class

//------------------------------------------------------------------------
//Get number of rows in result set
int ResultSet::count()
{
  PGresult *res = (PGresult *)pgres;
  return PQntuples(res);
}

//------------------------------------------------------------------------
//Get next row from result set
//Whether another was found - if so, clears and writes into row
bool ResultSet::fetch(Row& row)
{
  PGresult *res = (PGresult *)pgres;

  if (row_cursor < PQntuples(res))
  {
    // Load all the fields by name into the row
    int nf = PQnfields(res);
    row.clear();
    for(int i=0; i<nf; i++)
      row.add(PQfname(res, i), PQgetvalue(res, row_cursor, i));

    row_cursor++;
    return true;
  }
  else return false;
}

//------------------------------------------------------------------------
//Get first value of next row from result set
//Whether another was found - if so, writes into value
bool ResultSet::fetch(string& value)
{
  PGresult *res = (PGresult *)pgres;
  if (row_cursor < PQntuples(res) && PQnfields(res) > 0)
  {
    value = PQgetvalue(res, row_cursor++, 0);
    return true;
  }
  else return false;
}

//------------------------------------------------------------------------
//Destructor
ResultSet::~ResultSet() 
{
  PGresult *res = (PGresult *)pgres;
  PQclear(res);
}

//==========================================================================
//Postgres connection class

//------------------------------------------------------------------------
//Constructor - takes Postgres connection string
//e.g. "host=localhost dbname=foo user=prc password=secret"
Connection::Connection(const string& conninfo): 
  DB::Connection(), pgconn(0), last_oid(-1)
{
  PGconn *conn = PQconnectdb(conninfo.c_str());
  if (!conn || PQstatus(conn) == CONNECTION_BAD)
  {
    Log::Error << "DB: Cannot connect to PostgresQL at:\n";
    Log::Error << "[" << conninfo << "]\n";
    if (conn)
    {
      Log::Error << PQerrorMessage(conn) << endl;
      PQfinish(conn);
    }
    else Log::Error << "Can't allocate connection\n";
    return;
  }

  // OK, we have a connection
  Log::Detail << "PostgresQL connected\n";
  pgconn = conn;
  valid = true;
}

//------------------------------------------------------------------------
//Execute a command, not expecting any result (e.g. INSERT, UPDATE, DELETE)
//Returns whether successful
bool Connection::exec(const string& sql)
{
  PGconn *conn = (PGconn *)pgconn;

  if (Log::debug_ok) Log::Debug << "DBexec: " << sql << endl;

  PGresult *res = PQexec(conn, sql.c_str());

  if (!res)
  {
    Log::Error << "Postgres exec failed - NULL result\n";
    return false;
  }

  ExecStatusType status = PQresultStatus(res);
  if (status == PGRES_COMMAND_OK)
  {
    if (Log::debug_ok) Log::Debug << "DBexec OK" << endl;

    // Grab OID in case they want to see it
    Oid oid = PQoidValue(res);
    if (oid != InvalidOid)
      last_oid = (int)oid;
    else
      last_oid = -1;

    PQclear(res);
    return true;
  }
  else
  {
    Log::Error << "Postgres exec failed (" << PQresStatus(status) << "):\n";
    Log::Error << "  " << sql << endl;
    Log::Error << "  " << PQerrorMessage(conn);
    PQclear(res);
    return false;
  }
}

//------------------------------------------------------------------------
//Execute a query and get result (e.g. SELECT)
//Returns result - check this for validity
Result Connection::query(const string& sql)
{
  PGconn *conn = (PGconn *)pgconn;

  if (Log::debug_ok) Log::Debug << "DBquery: " << sql << endl;

  PGresult *res = PQexec(conn, sql.c_str());

  if (!res)
  {
    Log::Error << "Postgres query failed - NULL result\n";
    return Result();
  }

  ExecStatusType status = PQresultStatus(res);
  if (status == PGRES_TUPLES_OK)
  {
    if (Log::debug_ok) 
      Log::Debug << "DBquery OK: " << PQntuples(res) << " rows\n";
    return Result(new ResultSet(res));
  }
  else
  {
    Log::Error << "Postgres query failed (" << PQresStatus(status) << "):\n";
    Log::Error << "  " << sql << endl;
    Log::Error << "  " << PQerrorMessage(conn);
    PQclear(res);
    return Result();
  }
}

//------------------------------------------------------------------------
//Get integer ID of last INSERT, or -1 if none
int Connection::inserted_id()
{
  return last_oid;
}

//------------------------------------------------------------------------
//Destructor
Connection::~Connection() 
{
  PGconn *conn = (PGconn *)pgconn;
  if (valid && conn) PQfinish(conn);
}

}}} // namespaces



