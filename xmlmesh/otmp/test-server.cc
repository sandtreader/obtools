//==========================================================================
// ObTools::XMLBus: test-server.cc
//
// Test harness for raw OTMP server
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmlbus-otmp.h"
#include "ot-log.h"

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  // Set up logging
  Log::StreamChannel   chan_out(cout);
  Log::TimestampFilter tsfilter("%H:%M:%S %a %d %b %Y: ", chan_out);
  Log::LevelFilter     level_out(Log::LEVEL_DUMP, tsfilter);
  Log::logger.connect(level_out);

  // Create server and run it
  MT::Queue<XMLBus::OTMPMessage> q;
  XMLBus::OTMPServer server(q);
  server.run();
  return 0;  
}




