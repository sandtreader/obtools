//==========================================================================
// ObTools::Daemon: ot-daemon.h
//
// Public definitions for ObTools::Daemon
// Common infrastructure for Unix daemons/background servers
// 
// Copyright (c) 2009 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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
  string default_config_file;  // Path to default config if not specified 
  string config_element;       // Top-level element to expect in config
  string default_log_file;     // Default log file path
  string pid_file;             // PID file path

protected:
  bool enable_watchdog;        // Flag to enable watchdog
  bool shut_down;              // Shut down requested
  int slave_pid;               // PID of slave

  //--------------------------------------------------------------------------
  // Prerun function for child process
  // Called before priviledges dropped
  // Return exit code or 0 to continue
  virtual int run_priv()=0;

  //--------------------------------------------------------------------------
  // Main run function for child process
  // Return child exit code
  virtual int run()=0;

public:
  XML::Configuration config;  // Daemon-wide configuration

  //--------------------------------------------------------------------------
  // Constructor
  Process(const string& _default_config_file, const string& _config_element,
	  const string& _default_log_file, const string& _pid_file): 
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
  // Handle a failure signal
  void log_evil(int sig);
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_DAEMON_H



