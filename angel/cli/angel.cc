//==========================================================================
// ObTools::Angel:cli: angel.cc
//
// Main file for angel CLI utility
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xml.h"
#include "ot-log.h"
#include <fstream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  // Grab config filename if specified
  XML::Configuration config;
  if (argc > 1)
    config.add_file(argv[argc-1]);  // Last arg, leaves room for options
  else
  {
    // Option of local or /etc
    config.add_file("angel.cfg.xml");
    config.add_file("/etc/angel/angel.cfg.xml");
  }

  if (!config.read("angel"))
  {
    cerr << "Can't read configuration file\n";
    return 2;
  }

  // Set up logging
  Log::StreamChannel chan_out(cout);
  int log_level = config.get_value_int("log/@level", Log::LEVEL_SUMMARY);
  Log::LevelFilter level_out((Log::Level)log_level, chan_out);
  Log::logger.connect(level_out);
  Log::Streams log;

  log.summary << "This is angel\n";

  return 0;  
}




