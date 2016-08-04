//==========================================================================
// ObTools::Script: test-script.cc
//
// Test harness for XML script library
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-script.h"
#include "ot-log.h"
using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cerr << "Specify a script file\n";
    return 2;
  }

  Log::StreamChannel chan_out(cout);
  Log::TimestampFilter tsfilter("%H:%M:%S: ", chan_out);
  Log::logger.connect(tsfilter);
  Log::Streams log;

  // Create language
  Script::BaseLanguage language;

  // Read script
  ObTools::XML::Configuration config(argv[1]);
  if (!config.read("script")) return 2;
  XML::Element& root = config.get_root();
  Script::Script script(language, root);

  // Run script slowly, manually, with tick markers
  log.summary << "Starting script\n";
  for(;;)
  {
    log.detail << "--- tick ---\n";
    if (!script.tick()) break;
    this_thread::sleep_for(chrono::seconds{1});
  }
  log.summary << "Script finished\n";

  return 0;
}




