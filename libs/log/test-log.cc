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
  ObTools::Log::StreamChannel *chan1 = 
    new ObTools::Log::StreamChannel(cout, ObTools::Log::LEVEL_DUMP, "H*");
  ObTools::Log::StreamChannel *chan2 = 
    new ObTools::Log::StreamChannel(cerr, ObTools::Log::LEVEL_ERROR);

  ObTools::Log::logger.connect("cout", chan1);
  ObTools::Log::logger.connect("cerr", chan2);

  ObTools::Log::Summary << "Hello, world\nThis is a test\n\n";
  ObTools::Log::Summary << "You shouldn't see this\n";

  if (ObTools::Log::dump_ok)
    ObTools::Log::Dump << "This is more than you ever wanted to know\n";

  ObTools::Log::Error << "Hey, both cout and cerr should see this\n";
  ObTools::Log::Error << "But only cerr will see this\n";

  return 0;  
}


