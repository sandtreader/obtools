//==========================================================================
// ObTools::XMLMesh::Listener: listener.h
//
// Internal definitions for XMLMesh listener
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_XMLMESH_LISTENER_H
#define __OBTOOLS_XMLMESH_LISTENER_H

#include "ot-daemon.h"
#include "ot-time.h"
#include "ot-web.h"
#include "ot-init.h"
#include "ot-xmlmesh-client-otmp.h"

namespace ObTools { namespace XMLMesh {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ObTools;

//==========================================================================
// Global state
// Singleton instance of server-wide state
class Server: public Daemon::Application
{
 private:
  // Configuration read from file
  XML::Element config_xml;

 public:
  // Mesh interface
  XMLMesh::OTMPMultiClient *mesh;

  //------------------------------------------------------------------------
  // Constructor
  Server();

  //------------------------------------------------------------------------
  // Read settings from configuration
  virtual void read_config(const XML::Configuration& config);

  //------------------------------------------------------------------------
  // Prerun function for child process
  virtual int run_priv();

  //------------------------------------------------------------------------
  // Pre main loop function
  virtual int pre_run();

  //------------------------------------------------------------------------
  // Main loop iteration function
  virtual int tick();

  //------------------------------------------------------------------------
  // Preconfigure from config file, before daemon() called
  int preconfigure();

  //------------------------------------------------------------------------
  // Global configuration - called only at startup
  // Returns whether successful
  bool configure();

  //------------------------------------------------------------------------
  // Global re-configuration - called at startup and also at SIGHUP
  void reconfigure();

  //------------------------------------------------------------------------
  // Clean up
  void cleanup();
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLMESH_LISTENER_H
