//==========================================================================
// ObTools::Log: test-log.cc
//
// Test harness for log library
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-log.h"
using namespace std;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  ObTools::Log::StreamChannel chan_out(cout);
  ObTools::Log::StreamChannel chan_err(cerr);

  ObTools::Log::PatternFilter hfilter("H*", chan_out);
  ObTools::Log::LevelFilter   level_out(ObTools::Log::LEVEL_SUMMARY, hfilter);

  ObTools::Log::TimestampFilter tsfilter("%H:%M:%S %a %d %b %Y: ", chan_err);
  ObTools::Log::LevelFilter     level_err(ObTools::Log::LEVEL_ERROR, tsfilter);

  ObTools::Log::logger.connect(hfilter);
  ObTools::Log::logger.connect(level_err);

  ObTools::Log::Summary << "Hello, world\nThis is a test\n\n";
  ObTools::Log::Summary << "You shouldn't see this\n";

  if (ObTools::Log::dump_ok)
    ObTools::Log::Dump << "This is more than you ever wanted to know\n";

  ObTools::Log::Error << "Hey, both cout and cerr should see this\n";
  ObTools::Log::Error << "But only cerr will see this\nand this\n";

  return 0;  
}


