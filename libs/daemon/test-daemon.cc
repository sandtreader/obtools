//==========================================================================
// ObTools::Daemon: test-daemon.cc
//
// Test harness for daemon library
//
// Copyright (c) 2009 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
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
  // Preconfigure function for child process
  int preconfigure()
  {
    Log::Streams log;
    log.summary << "Hi, this is the pre-daemon() preconfigure\n";
    return 0;
  }

  //--------------------------------------------------------------------------
  // Prerun function for child process
  int run_priv()
  {
    Log::Streams log;
    log.summary << "Hi, this is the privileged prerun\n";
    return 0;
  }

  //--------------------------------------------------------------------------
  // Reconfigure process
  void reconfigure()
  {
    Log::Streams log;
    log.summary << "This is the reconfigure function\n";
  }

  //--------------------------------------------------------------------------
  // Main run function for child process
  // Return child exit code
  int run()
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
  TestDaemon(): Daemon::Process("Daemon library test", "0.1",
				"test.cfg", "test", 
				"/tmp/test.log", "/var/run/test.pid") 
  {
  }
};

TestDaemon test_daemon;

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  return test_daemon.start(argc, argv);
}




