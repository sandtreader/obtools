//==========================================================================
// ObTools::XMLMesh:Server: main.cc
//
// Main entry point for XMLMesh Server
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "server.h"
#include "ot-log.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::XMLMesh;

// Global server instance
Server ObTools::XMLMesh::server;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  // Run initialisation sequence (auto-registration of modules etc.)
  Init::Sequence::run();

  // Read config
  char *cfg = "xmlmesh.cfg.xml";
  if (argc > 1) cfg = argv[1];
  XML::Configuration config(cfg);
  if (!config.read("xmlmesh"))
  {
    Log::Error << "Can't read configuration file\n";
    return 2;
  }

  // Set up logging
  Log::StreamChannel   chan_out(cout);
  Log::TimestampFilter tsfilter("%H:%M:%S %a %d %b %Y: ", chan_out);
  int log_level = config.get_value_int("log/@level", Log::LEVEL_SUMMARY);
  Log::LevelFilter level_out((Log::Level)log_level, tsfilter);
  Log::logger.connect(level_out);
  Log::Summary << "xmlmesh-server starting\n";
  
  // Configure server 
  server.configure(config);

  // Run server (never returns)
  server.run();

  return 0;  
}




