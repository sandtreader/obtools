//==========================================================================
// ObTools::Daemon: ot-daemon.h
//
// Public definitions for ObTools::Daemon
// Common infrastructure for Unix daemons/background servers
// 
// Copyright (c) 2009 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_DAEMON_H
#define __OBTOOLS_DAEMON_H

#include "ot-net.h"
#include "ot-mt.h"
#include "ot-log.h"
#include "ot-xml.h"

namespace ObTools { namespace Daemon { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Process class - manages the process, forks and monitors a child, sets up
// logging from config, etc.
// Abstract: Implement run() in daemon-specific subclass
class Process
{
  string name;                 // Long name of process
  string version;              // Version number
  string default_config_file;  // Path to default config if not specified 
  string config_element;       // Top-level element to expect in config
  string default_log_file;     // Default log file path
  string pid_file;             // PID file path

protected:
  bool enable_watchdog;        // Flag to enable watchdog
  bool shut_down;              // Shut down requested
  int slave_pid;               // PID of slave or 0 if we are the slave

  //--------------------------------------------------------------------------
  // Preconfigure function for child process, called before daemon()
  // Does nothing by default - override to implement
  // Return exit code or 0 to continue
  virtual int preconfigure() { return 0; }

  //--------------------------------------------------------------------------
  // Prerun function for child process
  // Called before privileges dropped
  // Does nothing by default - override to implement
  // Return exit code or 0 to continue
  virtual int run_priv() { return 0; }

  //--------------------------------------------------------------------------
  // Reconfigure function for child process
  // Does nothing by default - override to implement
  // Called on reception of SIGHUP by master or child
  virtual void reconfigure() {}

  //--------------------------------------------------------------------------
  // Main run function for child process
  // Return child exit code
  virtual int run()=0;

public:
  XML::Configuration config;  // Daemon-wide configuration

  //--------------------------------------------------------------------------
  // Constructor
 Process(const string& _name, const string& _version,
	 const string& _default_config_file, const string& _config_element,
	 const string& _default_log_file, const string& _pid_file): 
    name(_name), version(_version),
    default_config_file(_default_config_file), 
    config_element(_config_element),
    default_log_file(_default_log_file),
    pid_file(_pid_file), 
#ifdef DEBUG
    enable_watchdog(false),
#else
    enable_watchdog(true),
#endif
    shut_down(false), slave_pid(0)
  {}

  //--------------------------------------------------------------------------
  // Start process, with arguments
  // Returns process exit code
  int start(int argc, char **argv);

  //--------------------------------------------------------------------------
  // Signal to shut down 
  void shutdown();

  //--------------------------------------------------------------------------
  // Signal to reload config
  void reload();

  //--------------------------------------------------------------------------
  // Handle a failure signal
  void log_evil(int sig);
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_DAEMON_H


