//==========================================================================
// ObTools::DB: db-mysql.cc
//
// MySQL Database wrapper implementation
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-db-mysql.h"
#include "ot-log.h"
#include "mysql.h"

namespace ObTools { namespace DB { namespace MySQL {

//==========================================================================
//MySQL result class

//------------------------------------------------------------------------
//Constructor
ResultSet::ResultSet(void *_myres): myres(_myres)
{
  MYSQL_RES *res = (MYSQL_RES *)myres;

  // Get field definitions
  mynum_fields = mysql_num_fields(res);
  myfields = mysql_fetch_fields(res);
}

//------------------------------------------------------------------------
//Get number of rows in result set
int ResultSet::count()
{
  MYSQL_RES *res = (MYSQL_RES *)myres;
  return mysql_num_rows(res);
}

//------------------------------------------------------------------------
//Get next row from result set
//Whether another was found - if so, clears and writes into row
bool ResultSet::fetch(Row& row)
{
  MYSQL_RES *res = (MYSQL_RES *)myres;

  // Try to fetch a row
  MYSQL_ROW myrow = mysql_fetch_row(res);

  if (myrow)
  {
    row.clear();

    // Load all the fields by name into the row, unescaping as we go
    MYSQL_FIELD *fields = (MYSQL_FIELD *)myfields;
    for(unsigned int i=0; i<mynum_fields; i++)
      if (myrow[i]) row.add_unescaped(fields[i].name, myrow[i]);

    return true;
  }
  else return false;
}

//------------------------------------------------------------------------
//Get first value of next row from result set
//Value is unescaped
//Whether another was found - if so, writes into value
bool ResultSet::fetch(string& value)
{
  MYSQL_RES *res = (MYSQL_RES *)myres;

  // Try to fetch a row
  MYSQL_ROW myrow = mysql_fetch_row(res);

  if (myrow && mynum_fields > 0)
  {
    value = Row::unescape(myrow[0]);
    return true;
  }
  else return false;
}

//------------------------------------------------------------------------
//Destructor
ResultSet::~ResultSet() 
{
  MYSQL_RES *res = (MYSQL_RES *)myres;
  mysql_free_result(res);
}

//==========================================================================
//MySQL connection class

//------------------------------------------------------------------------
//Constructor 
Connection::Connection(const string& host, const string& user,
		       const string& passwd, const string& dbname,
		       unsigned int port): 
  DB::Connection(), myconn(0)
{
  MYSQL *conn = mysql_init(0);

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
    myconn = conn;
    valid = true;
  }
}

//------------------------------------------------------------------------
//Execute a command, not expecting any result (e.g. INSERT, UPDATE, DELETE)
//Returns whether successful
bool Connection::exec(const string& sql)
{
  MYSQL *conn = (MYSQL *)myconn;

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

//------------------------------------------------------------------------
//Execute a query and get result (e.g. SELECT)
//Returns result - check this for validity
Result Connection::query(const string& sql)
{
  MYSQL *conn = (MYSQL *)myconn;

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

//------------------------------------------------------------------------
//Destructor
Connection::~Connection() 
{
  MYSQL *conn = (MYSQL *)myconn;
  if (valid && conn) mysql_close(conn);
}

}}} // namespaces



