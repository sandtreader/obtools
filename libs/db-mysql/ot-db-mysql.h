//==========================================================================
// ObTools::DB: ot-db-mysql.h
//
// Definition of MySQL-specific database driver
// 
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_DB_MYSQL_H
#define __OBTOOLS_DB_MYSQL_H

#include "ot-db.h"
#include "ot-log.h"

namespace ObTools { namespace DB { namespace MySQL {  

//Make our lives easier without polluting anyone else
using namespace std;

// Note we use void *'s here to avoid forcing our uses to include mysql.h

//==========================================================================
//MySQL result class
class ResultSet: public DB::ResultSet
{ 
private:
  void *myres;  // MySQL result structure (MYSQL_RES *)

  // Field definitions
  unsigned int mynum_fields;
  void *myfields; // MYSQL_FIELD *

public:
  //------------------------------------------------------------------------
  //Constructor
  ResultSet(void *res);

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
//MySQL connection class
class Connection: public DB::Connection
{
private:
  void *myconn;      // MySQL connection structure (MYSQL *)
  Log::Streams log; // Private (therefore per-thread, assuming connections
                    // are not shared) log streams

public:
  //------------------------------------------------------------------------
  //Constructor 
  Connection(const string& host, const string& user,
	     const string& passwd, const string& dbname,
	     unsigned int port=0);

  //------------------------------------------------------------------------
  //Execute a command, not expecting any result (e.g. INSERT, UPDATE, DELETE)
  //Returns whether successful
  bool exec(const string& sql);

  //------------------------------------------------------------------------
  //Execute a query and get result (e.g. SELECT)
  //Returns result - check this for validity
  Result query(const string& sql);

  //------------------------------------------------------------------------
  //Destructor
  ~Connection();
};

//==========================================================================
}}} //namespaces
#endif // !__OBTOOLS_DB_MYSQL_H



