//==========================================================================
// ObTools::DB: db-mysql.cc
//
// MySQL Database wrapper implementation
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-db-mysql.h"
#include "ot-log.h"

namespace ObTools { namespace DB { namespace MySQL {

//==========================================================================
// MySQL result class

//--------------------------------------------------------------------------
// Constructor
ResultSet::ResultSet(MYSQL_RES *_res): res{_res},
  num_fields{mysql_num_fields(res)}, fields{mysql_fetch_fields(res)}
{}

//--------------------------------------------------------------------------
// Get number of rows in result set
int ResultSet::count()
{
  return mysql_num_rows(res);
}

//--------------------------------------------------------------------------
// Get next row from result set
// Whether another was found - if so, clears and writes into row
bool ResultSet::fetch(Row& row)
{
  // Try to fetch a row
  MYSQL_ROW myrow = mysql_fetch_row(res);

  if (myrow)
  {
    row.clear();

    // Load all the fields by name into the row
    for (auto i = 0u; i < num_fields; ++i)
      if (myrow[i]) row.add(fields[i].name, myrow[i]);

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
  // Try to fetch a row
  MYSQL_ROW myrow = mysql_fetch_row(res);

  if (myrow && num_fields > 0)
  {
    value = myrow[0] ? myrow[0] : "";
    return true;
  }
  else return false;
}

//--------------------------------------------------------------------------
// Destructor
ResultSet::~ResultSet()
{
  mysql_free_result(res);
}

//==========================================================================
// MySQL connection class

//--------------------------------------------------------------------------
// Constructor
Connection::Connection(const string& host, const string& user,
                       const string& passwd, const string& dbname,
                       unsigned int port)
{
  conn = mysql_init(0);

  if (conn)
  {
    if (!mysql_real_connect(conn, host.c_str(), user.c_str(), passwd.c_str(),
                            dbname.c_str(), port, 0, 0))
    {
      log.error << "DB: Can't connect to MySQL on " << host << ": "
                << mysql_error(conn) << endl;
      mysql_close(conn);
      conn = 0;
    }
  }
  else log.error << "Can't allocate MySQL connection\n";

  if (conn)
  {
    // OK, we have a connection
    log.detail << "MySQL connected to " << dbname << " on " << host << endl;
  }
}

//--------------------------------------------------------------------------
// Check whether connection is OK
Connection::operator bool()
{
  if (!conn)
    return false;
  if (!mysql_ping(conn)) return true;

  log.error << "MySQL connection failed: " << mysql_error(conn) << endl;
  return false;
}

//--------------------------------------------------------------------------
// Execute a command, not expecting any result (e.g. INSERT, UPDATE, DELETE)
// Returns whether successful
bool Connection::exec(const string& sql)
{
  OBTOOLS_LOG_IF_DEBUG(log.debug << "DBexec: " << sql << endl;)

  if (mysql_query(conn, sql.c_str()))
  {
    log.error << "MySQL exec failed: " << mysql_error(conn) << endl;
    return false;
  }

  OBTOOLS_LOG_IF_DEBUG(log.debug << "DBexec OK, "
                       << mysql_affected_rows(conn) << " rows affected\n";)
  return true;
}

//--------------------------------------------------------------------------
// Execute a query and get result (e.g. SELECT)
// Returns result - check this for validity
Result Connection::query(const string& sql)
{
  OBTOOLS_LOG_IF_DEBUG(log.debug << "DBquery: " << sql << endl;)

  if (mysql_query(conn, sql.c_str()))
  {
    log.error << "MySQL query failed: " << mysql_error(conn) << endl;
    return Result();
  }

  // Get result structure - stored locally
  MYSQL_RES *res = mysql_store_result(conn);

  if (!res)
  {
    log.error << "MySQL query returned no result: ";
    log.error << sql << endl;
    return Result();
  }

  OBTOOLS_LOG_IF_DEBUG(log.debug << "DBquery OK: "
                       << mysql_num_rows(res) << " rows\n";)
  return Result(new ResultSet(res));
}

//--------------------------------------------------------------------------
// Gets the last insert id
uint64_t Connection::get_last_insert_id()
{
  return mysql_insert_id(conn);
}

//--------------------------------------------------------------------------
// Destructor
Connection::~Connection()
{
  if (conn) mysql_close(conn);
}

}}} // namespaces
