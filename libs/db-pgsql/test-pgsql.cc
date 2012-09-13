//==========================================================================
// ObTools::DB: test-pgsql.cc
//
// Test harness for Postgres database driver
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
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
  Log::Streams log;

  // Connect database
//  DB::PG::Connection conn("host=testhost dbname=test user=postgres");
  DB::PG::Connection conn("host=testhost  dbname=postgres user=postgres");
  if (!conn) return 2;

  conn.exec("THIS DOESN'T WORK");
  conn.exec("DROP table  test");
  if (!conn.exec("CREATE table test (id int, name varchar(256) )")) return 2;
  if (!conn.exec("INSERT into test values(3, 'Fred')")) return 2;
  if (!conn.exec("INSERT into test values(4, 'Jim')")) return 2;
  if (!conn.exec("INSERT into test values(5, 'Pete')")) return 2;

  DB::Result res = conn.query("SELECT * from test");
  if (!res) return 2;

  DB::Row row;
  while (res.fetch(row))
    log.detail << row["id"] << ":" << row["name"] << endl;

  if (!conn.exec("DELETE from test where id=3")) return 2;

  return 0;  
}




