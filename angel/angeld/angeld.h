//==========================================================================
// ObTools::Angel:angeld: angeld.h
//
// Internal definitions for angeld daemon
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_ANGEL_DAEMON_H
#define __OBTOOLS_ANGEL_DAEMON_H

#include <string>
#include "ot-net.h"
#include "ot-log.h"
#include "ot-mt.h"
#include "ot-cache.h"
#include "ot-xmlmesh-client.h"
#include "ot-init.h"

namespace ObTools { namespace Angel {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Process object
struct Process
{
  string id;                    // Unique ID
  string name;                  // Descriptive name
  string command;               // Command to run, including arguments

  // Dependency DAG 
  list<string> depend_ids;      // List of dependency ids (loaded first)
  list<Process *> dependants;   // Processes that depend on us
  list<Process *> dependencies; // Processes that we depend on

  // State
  bool mark;                    // Mark to detect loops
  bool started;                 // Process started

  //------------------------------------------------------------------------
  // Constructor from XML config 
  Process(XML::Configuration& config);

  //------------------------------------------------------------------------
  // Add dependency of this on another process
  void add_dependency(Process *dependency) 
  { dependencies.push_back(dependency); }

  //------------------------------------------------------------------------
  // Add dependency on this from another process
  void add_dependant(Process *dependant) 
  { dependants.push_back(dependant); }

  //------------------------------------------------------------------------
  // Start process
  // Returns whether successful
  bool start();

};

//==========================================================================
// Main angeld server object
class Server: public XML::Configuration
{
private:
  Log::Streams log;                         // Foreground logging
  list<Process *> processes;                // Master list of processes
  map<string, Process *> process_ids;       // Secondary map by id

  bool load_processes();
  Process *lookup(const string& id);
  void create_dependencies();
  void start_processes();

public:
  XMLMesh::MultiClient *mesh;               // Mesh connection

  //------------------------------------------------------------------------
  // Constructor 
  Server() {}

  //------------------------------------------------------------------------
  // Load config from XML
  bool configure();

  //------------------------------------------------------------------------
  // Run method - never returns
  void run(); 

  //------------------------------------------------------------------------
  // Destructor
  ~Server();
};

// Global server instance
extern Server server;

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_ANGEL_DAEMON_H



