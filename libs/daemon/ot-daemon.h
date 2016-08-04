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

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Application class - an abstract base class for an application's running
// process
class Application
{
public:
  //------------------------------------------------------------------------
  // Read configuration
  virtual void read_config(const XML::Configuration&) {}

  //------------------------------------------------------------------------
  // Preconfigure function
  // Return exit code or 0 to continue
  virtual int preconfigure() { return 0; }

  //------------------------------------------------------------------------
  // Called before privileges are dropped
  // Return exit code or 0 to continue
  virtual int run_priv() { return 0; }

  //------------------------------------------------------------------------
  // Reconfigure function
  virtual void reconfigure() {}

  //------------------------------------------------------------------------
  // Prerun function
  // Called before main loop (tick()s)
  // Return exit code or 0 to continue
  virtual int pre_run() { return 0; }

  //------------------------------------------------------------------------
  // Time to sleep until next tick
  // Return value in microseconds
  virtual int tick_wait() { return 10000; }

  //------------------------------------------------------------------------
  // Main function iteration
  // Return exit code or 0 to continue
  virtual int tick() { return 0; }

  //------------------------------------------------------------------------
  // Clean up function
  virtual void cleanup() {}

  //------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Application() {}
};

//==========================================================================
// Shell class - manages the application, forks and monitors a child, sets
// up logging from config, etc.
class Shell
{
  Application& application;     // The application to manage
  string name;                 // Long name of process
  string version;              // Version number
  string default_config_file;  // Path to default config if not specified
  string config_element;       // Top-level element to expect in config
  string default_log_file;     // Default log file path
  string default_pid_file;     // Default PID file path

  // Internal
  int drop_privileges();

protected:
  bool shut_down;              // Shut down requested
  int slave_pid;               // PID of slave or 0 if we are the slave

  //------------------------------------------------------------------------
  // Main run function for child process
  // Return child exit code
  virtual int run();

public:
  XML::Configuration config;  // Daemon-wide configuration

  //------------------------------------------------------------------------
  // Constructor
 Shell(Application& _application, const string& _name, const string& _version,
       const string& _default_config_file, const string& _config_element,
       const string& _default_log_file, const string& _default_pid_file):
    application(_application),
    name(_name), version(_version),
    default_config_file(_default_config_file),
    config_element(_config_element),
    default_log_file(_default_log_file),
    default_pid_file(_default_pid_file),
    shut_down(false), slave_pid(0),
    config(cerr, XML::PARSER_OPTIMISE_CONTENT
           | XML::PARSER_PRESERVE_WHITESPACE)
  {}

  //------------------------------------------------------------------------
  // Start process, with arguments
  // Returns process exit code
  int start(int argc, char **argv);

  //------------------------------------------------------------------------
  // Signal to shut down
  void shutdown();

  //------------------------------------------------------------------------
  // Signal to reload config
  void reload();

  //------------------------------------------------------------------------
  // Handle a failure signal
  void log_evil(int sig);

  //------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Shell() {}
};

//==========================================================================
// Process class - manages the process, forks and monitors a child, sets up
// logging from config, etc.
// Abstract: Implement run() in daemon-specific subclass
class Process: public Shell, public Application
{
protected:
  //------------------------------------------------------------------------
  // Preconfigure function for child process, called before daemon()
  // Does nothing by default - override to implement
  // Return exit code or 0 to continue
  virtual int preconfigure() { return 0; }

  //------------------------------------------------------------------------
  // Prerun function for child process
  // Called before privileges dropped
  // Does nothing by default - override to implement
  // Return exit code or 0 to continue
  virtual int run_priv() { return 0; }

  //------------------------------------------------------------------------
  // Reconfigure function for child process
  // Does nothing by default - override to implement
  // Called on reception of SIGHUP by master or child
  virtual void reconfigure() {}

  //------------------------------------------------------------------------
  // Optional cleanup function, called after run() exits
  virtual void cleanup() {};

public:
  //------------------------------------------------------------------------
  // Constructor
 Process(const string& _name, const string& _version,
         const string& _default_config_file, const string& _config_element,
         const string& _default_log_file, const string& _default_pid_file):
    Shell(*this, _name, _version, _default_config_file, _config_element,
          _default_log_file, _default_pid_file)
  {}

  //------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Process() {}
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_DAEMON_H



