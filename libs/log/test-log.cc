//==========================================================================
// ObTools::Log: test-log.cc
//
// Test harness for log library
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-log.h"
using namespace std;
using namespace ObTools::Log;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  StreamChannel chan_out(cout);
  StreamChannel chan_err(cerr);

  PatternFilter hfilter("H*", chan_out);
  LevelFilter   level_out(LEVEL_SUMMARY, hfilter);

  TimestampFilter tsfilter("%H:%M:%S %a %d %b %Y: ", chan_err);
  LevelFilter     level_err(LEVEL_ERROR, tsfilter);

  logger.connect(hfilter);
  logger.connect(level_err);

  Summary << "Hello, world\nThis is a test\n\n";
  Summary << "You shouldn't see this\n";

  if (dump_ok)
    Dump << "This is more than you ever wanted to know\n";

  Error << "Hey, both cout and cerr should see this\n";
  Error << "But only cerr will see this\nand this\n";

  return 0;  
}


