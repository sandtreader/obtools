//==========================================================================
// ObTools::XMLBus:Server: main.cc
//
// Main entry point for XMLBus Server
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "server.h"
#include "transport-otmp.h"
#include "ot-log.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::XMLBus;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  char *cfg = "xmlbus.cfg.xml";
  if (argc > 1) cfg = argv[1];

  // Set up logging
  Log::StreamChannel   chan_out(cout);
  Log::TimestampFilter tsfilter("%H:%M:%S %a %d %b %Y: ", chan_out);
  Log::LevelFilter     level_out(Log::LEVEL_DUMP, tsfilter);
  Log::logger.connect(level_out);

  // Read config
  XML::Configuration config(cfg);
  if (!config.read("xmlbus"))
  {
    Log::Error << "Can't read configuration file\n";
    return 2;
  }

  // Create server 
  Server server;

  // Register transport modules
  OTMPServerTransportFactory::register_into(server);

  // Configure server 
  server.configure(config);

  // Run server (never returns)
  server.run();

  return 0;  
}




