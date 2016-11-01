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
#include "ot-xmlmesh-client.h"

namespace ObTools { namespace XMLMesh {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ObTools;

//==========================================================================
// A listener action - mapping from subject pattern to a command to run
class Action
{
  string command;  // Command to run, with expansion $SUBJECT

  // Subscriber class
  class Subscriber: public XMLMesh::Subscriber
  {
    Action& action;

    // Implementation of subscriber message handling
    void handle(XMLMesh::Message& msg) { action.handle(msg); }

  public:
    Subscriber(XMLMesh::MultiClient& mesh,
               const string& subject,
               Action& _action):
      XMLMesh::Subscriber(mesh, subject), action(_action) {}

    XMLMesh::MultiClient& get_mesh() { return client; }
  };

  // Note - explicitly managed because we live in a map and we can't have it
  // being unsubscribed just because the map wants to do some shuffling
  Subscriber *subscriber{nullptr};

public:
  //------------------------------------------------------------------------
  // Constructors
  Action() {}
  Action(const string& _command):
    command(_command) {}

  //------------------------------------------------------------------------
  // Explicit update from a new action, leaving subscriber in place
  void update(const Action& o) { command = o.command; }

  //------------------------------------------------------------------------
  // Comparator
  bool operator==(const Action& o) { return o.command == command; }

  //------------------------------------------------------------------------
  // Handle a message
  void handle(const XMLMesh::Message& msg);

  //------------------------------------------------------------------------
  // Subscribe to the given subject
  void subscribe(XMLMesh::MultiClient& mesh, const string& subject);

  //------------------------------------------------------------------------
  // Unsubscribe
  void unsubscribe();
};

//==========================================================================
// Global state
// Singleton instance of server-wide state
class Server: public Daemon::Application
{
 private:
  // Configuration read from file
  XML::Element config_xml;

  // List of current actions, by subject
  MT::RWMutex actions_mutex;
  map<string, Action> actions;

 public:
  // Mesh interface
  XMLMesh::MultiClient *mesh;

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
