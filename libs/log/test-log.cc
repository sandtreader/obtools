//==========================================================================
// ObTools::Log: test-log.cc
//
// Test harness for log library
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-log.h"
using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main()
{
  Log::StreamChannel chan_out(cout);
  Log::StreamChannel chan_err(cerr);

  Log::PatternFilter hfilter("H*", chan_out);
  Log::LevelFilter   level_out(Log::LEVEL_SUMMARY, hfilter);

  Log::TimestampFilter tsfilter("%H:%M:%S %a %d %b %Y: ", chan_err);
  Log::LevelFilter     level_err(Log::LEVEL_ERROR, tsfilter);

  Log::logger.connect(hfilter);
  Log::logger.connect(level_err);

  Log::Summary << "Hello, world\nThis is a test\n\n";
  Log::Summary << "You shouldn't see this\n";

  if (Log::dump_ok)
    Log::Dump << "This is more than you ever wanted to know\n";

  Log::Error << "Hey, both cout and cerr should see this\n";
  Log::Error << "But only cerr will see this\nand this\n";

  return 0;  
}


