//==========================================================================
// ObTools::DB: ot-db-mysql.h
//
// Definition of MySQL-specific database driver
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_DB_MYSQL_H
#define __OBTOOLS_DB_MYSQL_H

#include "ot-db.h"
#include "ot-log.h"
#include <mysql/mysql.h>

namespace ObTools { namespace DB { namespace MySQL {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// MySQL result class
class ResultSet: public DB::ResultSet
{
private:
  MYSQL_RES *res = nullptr;  // MySQL result structure

  // Field definitions
  unsigned int num_fields = 0;
  MYSQL_FIELD *fields = nullptr;

public:
  //------------------------------------------------------------------------
  // Constructor
  ResultSet(MYSQL_RES *res);

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
// MySQL connection class
class Connection: public DB::Connection
{
private:
  MYSQL *conn = nullptr;  // MySQL connection structure
  Log::Streams log; // Private (therefore per-thread, assuming connections
                    // are not shared) log streams

public:
  //------------------------------------------------------------------------
  // Constructor
  Connection(const string& host, const string& user,
             const string& passwd, const string& dbname,
             unsigned int port=0);

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
  Statement prepare(const string& /* sql */) override
  {
    throw runtime_error("Prepare not implemented for MySQL");
  }

  //------------------------------------------------------------------------
  // Gets the last insert id
  uint64_t get_last_insert_id() override;

  //------------------------------------------------------------------------
  // Destructor
  ~Connection();
};

//==========================================================================
// MySQL connection factory
class ConnectionFactory: public DB::ConnectionFactory
{
  // Connection details
  string host;
  string user;
  string passwd;
  string dbname;
  unsigned int port;

  //------------------------------------------------------------------------
  // Interface to create a new connection
  DB::Connection *create_connection() override
  { return new MySQL::Connection(host, user, passwd, dbname, port); }

public:
  //------------------------------------------------------------------------
  // Constructor
  ConnectionFactory(const string& _host, const string& _user,
                    const string& _passwd, const string& _dbname,
                    unsigned int _port=0):
    host{_host}, user{_user}, passwd{_passwd}, dbname{_dbname}, port{_port}
  {}
  ConnectionFactory(const string& _host, const string& _user,
                    const string& _passwd, const string& _dbname,
                    unsigned int _port,
                    const map<string, string>& statements):
    DB::ConnectionFactory{statements},
    host{_host}, user{_user}, passwd{_passwd}, dbname{_dbname}, port{_port}
  {}
};


//==========================================================================
}}} //namespaces
#endif // !__OBTOOLS_DB_MYSQL_H
