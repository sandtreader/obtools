//==========================================================================
// ObTools::DB: test-mysql.cc
//
// Test harness for MySQL database driver
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-db-mysql.h"
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
  DB::MySQL::Connection conn("localhost", "", "", "test");
  if (!conn) return 2;

  conn.exec("THIS DOESN'T WORK");
  if (!conn.exec("DELETE from test where id=3")) return 2;
  if (!conn.exec("INSERT into test values(3, 'Fred')")) return 2;

  DB::Result res = conn.query("SELECT * from test");
  if (!res) return 2;

  DB::Row row;
  while (res.fetch(row))
    log.detail << row["id"] << ":" << row["name"] << endl;

  return 0;  
}



