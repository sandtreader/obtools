//==========================================================================
// ObTools::Angel:angeld: angeld.cc
//
// Main entry point for angeld daemon
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "angeld.h"
#include <fstream>
#include <errno.h>
#ifndef __WIN32__
#include <unistd.h>
#endif

#define DEFAULT_LOGFILE "/var/log/angel/angeld.log"
#define PID_FILE        "/var/run/angeld.pid"

using namespace std;
using namespace ObTools;
using namespace ObTools::Angel;

// Global server instance
Server ObTools::Angel::server;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  // Run initialisation sequence (auto-registration of modules etc.)
  Init::Sequence::run();

  // Grab config filename if specified
  if (argc > 1)
    server.add_file(argv[argc-1]);  // Last arg, leaves room for options
  else
  {
    // Option of local or /etc
    server.add_file("angeld.cfg.xml");
    server.add_file("/etc/angel/angeld.cfg.xml");
  }

  if (!server.read("angeld"))
  {
    cerr << "Can't read configuration file\n";
    return 2;
  }

  // Set up logging
#if defined(DAEMON)
  string logfile = server.get_value("log/@file", DEFAULT_LOGFILE);
  ofstream logstream(logfile.c_str(),ios::app);
  if (!logstream)
  {
    cerr << "angeld: Unable to open logfile " << logfile << endl;
    return 2;
  }
  Log::StreamChannel chan_out(logstream);
#else
  Log::StreamChannel chan_out(cout);
#endif
  Log::TimestampFilter tsfilter("%H:%M:%S %a %d %b %Y: ", chan_out);
  int log_level = server.get_value_int("log/@level", Log::LEVEL_SUMMARY);
  Log::LevelFilter level_out(static_cast<Log::Level>(log_level), tsfilter);
  Log::logger.connect(level_out);
  Log::Streams log;

#if defined(DAEMON)
  if (daemon(0, 0))
    log.error << "Can't become daemon: " << strerror(errno) << endl;

  // Create pid file
  ofstream pidfile(PID_FILE);
  pidfile << getpid() << endl;
  pidfile.close();
#endif

  log.summary << "angeld starting\n";
  
  // Configure server 
  if (!server.configure())
  {
    log.error << "Can't start angeld\n";
    return 2;
  }

  // Run server (never returns)
  server.run();

  return 0;  
}




