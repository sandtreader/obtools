//==========================================================================
// ObTools::DB: test-pgsql.cc
//
// Test harness for Postgres database driver
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#include "ot-db-pgsql.h"
#include "ot-log.h"
using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main()
{
  // Set up logging
  Log::StreamChannel   chan_out(cout);
  Log::TimestampFilter tsfilter("%H:%M:%S: ", chan_out);
  Log::LevelFilter level_out(Log::LEVEL_DEBUG, tsfilter);
  Log::logger.connect(level_out);

  // Connect database
  DB::PG::Connection conn("host=localhost dbname=test user=postgres");
  if (!conn) return 2;

  conn.exec("THIS DOESN'T WORK");
  if (!conn.exec("DELETE from test where id=3")) return 2;
  if (!conn.exec("INSERT into test values(3, 'Fred')")) return 2;

  DB::Result res = conn.query("SELECT * from test");
  if (!res) return 2;

  DB::Row row;
  while (res.fetch(row))
    Log::Detail << row["id"] << ":" << row["name"] << endl;

  return 0;  
}




