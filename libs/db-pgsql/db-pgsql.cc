//==========================================================================
// ObTools::DB: db-pgsql.cc
//
// PostgresQL Database wrapper implementation
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-db-pgsql.h"
#include "ot-log.h"

namespace ObTools { namespace DB { namespace PG {

//==========================================================================
// Postgres result class

//--------------------------------------------------------------------------
// Get number of rows in result set
int ResultSet::count()
{
  return PQntuples(res);
}

//--------------------------------------------------------------------------
// Get next row from result set
// Whether another was found - if so, clears and writes into row
bool ResultSet::fetch(Row& row)
{
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

//--------------------------------------------------------------------------
// Get first value of next row from result set
// Value is unescaped
// Whether another was found - if so, writes into value
bool ResultSet::fetch(string& value)
{
  if (row_cursor < PQntuples(res) && PQnfields(res) > 0)
  {
    value = PQgetvalue(res, row_cursor++, 0);
    return true;
  }
  else return false;
}

//--------------------------------------------------------------------------
// Destructor
ResultSet::~ResultSet()
{
  PQclear(res);
}

//==========================================================================
// Postgres connection class

//--------------------------------------------------------------------------
// Constructor - takes Postgres connection string
// e.g. "host=localhost dbname=foo user=prc password=secret"
Connection::Connection(const string& conninfo)
{
  conn = PQconnectdb(conninfo.c_str());
  if (!conn || PQstatus(conn) == CONNECTION_BAD)
  {
    log.error << "DB: Cannot connect to PostgresQL at:\n";
    log.error << "[" << conninfo << "]\n";
    if (conn)
    {
      log.error << PQerrorMessage(conn) << endl;
      PQfinish(conn);
    }
    else log.error << "Can't allocate connection\n";
    return;
  }

  // OK, we have a connection
  log.detail << "PostgresQL connected\n";
}

//--------------------------------------------------------------------------
// Check whether connection is OK
Connection::operator bool()
{
  if (!conn)
    return false;
  if (PQstatus(conn) == CONNECTION_OK) return true;

  log.error << "Postgres connection failed: " << PQerrorMessage(conn) << endl;
  return false;
}

//--------------------------------------------------------------------------
// Execute a command, not expecting any result (e.g. INSERT, UPDATE, DELETE)
// Returns whether successful
bool Connection::exec(const string& sql)
{
  OBTOOLS_LOG_IF_DEBUG(log.debug << "DBexec: " << sql << endl;)

  PGresult *res = PQexec(conn, sql.c_str());

  if (!res)
  {
    log.error << "Postgres exec failed - NULL result\n";
    return false;
  }

  ExecStatusType status = PQresultStatus(res);
  if (status == PGRES_COMMAND_OK)
  {
    OBTOOLS_LOG_IF_DEBUG(log.debug << "DBexec OK" << endl;)
    PQclear(res);
    return true;
  }
  else
  {
    log.error << "Postgres exec failed (" << PQresStatus(status) << "):\n";
    log.error << "  " << sql << endl;
    log.error << "  " << PQerrorMessage(conn);
    PQclear(res);
    return false;
  }
}

//--------------------------------------------------------------------------
// Execute a query and get result (e.g. SELECT)
// Returns result - check this for validity
Result Connection::query(const string& sql)
{
  OBTOOLS_LOG_IF_DEBUG(log.debug << "DBquery: " << sql << endl;)

  PGresult *res = PQexec(conn, sql.c_str());

  if (!res)
  {
    log.error << "Postgres query failed - NULL result\n";
    return Result();
  }

  ExecStatusType status = PQresultStatus(res);
  if (status == PGRES_TUPLES_OK)
  {
    OBTOOLS_LOG_IF_DEBUG(log.debug << "DBquery OK: "
                         << PQntuples(res) << " rows\n";)
    return Result(new ResultSet(res));
  }
  else
  {
    log.error << "Postgres query failed (" << PQresStatus(status) << "):\n";
    log.error << "  " << sql << endl;
    log.error << "  " << PQerrorMessage(conn);
    PQclear(res);
    return Result();
  }
}

//--------------------------------------------------------------------------
// Destructor
Connection::~Connection()
{
  if (conn) PQfinish(conn);
}

}}} // namespaces
