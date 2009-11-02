//==========================================================================
// ObTools::Daemon: test-daemon.cc
//
// Test harness for daemon library
//
// Copyright (c) 2009 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-daemon.h"
#include "ot-log.h"
#include <stdlib.h>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Test daemon class
class TestDaemon: public Daemon::Process
{
private:
  //--------------------------------------------------------------------------
  // Prerun function for child process
  virtual int run_priv()
  {
    Log::Streams log;
    log.summary << "Hi, this is the priviledged prerun\n";
    return 0;
  }

  //--------------------------------------------------------------------------
  // Main run function for child process
  // Return child exit code
  virtual int run()
  {
    Log::Streams log;
    log.summary << "Hello, this is the daemon run() method\n";
    log.detail << "My magic number is " << config["magic/@number"] << endl;

    // Sleep for 30 seconds unless shut down
    for(int i=0; i<30 && !shut_down; i++)
      MT::Thread::sleep(1);

    // Cause bad failure unless shut down
    if (!shut_down) abort();
    return 0;
  }

public:
  TestDaemon(): Daemon::Process("test.cfg", "test", 
				"/tmp/test.log", "/var/run/test.pid") 
  {
    // Force watchdog enabled, even in debug, for testing
    enable_watchdog = true;
  }
};

TestDaemon test_daemon;

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  return test_daemon.start(argc, argv);
}




