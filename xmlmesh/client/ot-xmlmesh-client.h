//==========================================================================
// ObTools::XMLMesh:Client: ot-xmlmesh-client.h
//
// Public definitions for XMLMesh client library
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_XMLMESH_CLIENT_H
#define __OBTOOLS_XMLMESH_CLIENT_H

#include <string>
#include "ot-net.h"
#include "ot-mt.h"
#include "ot-xmlmesh.h"

namespace ObTools { namespace XMLMesh {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Client Transport (abstract interface)
// Low-level transport of raw data
class ClientTransport
{
public:
  // Send a message - returns whether successful
  virtual bool send(const string& data) = 0;

  // Check whether a message is available before blocking in wait()
  virtual bool poll() = 0;

  // Wait for a message - blocking.
  // Returns whether one read - only returns false if something fails
  virtual bool wait(string& data) = 0;
};

//==========================================================================
// General XML Bus client using any transport
class Client
{
private:
  ClientTransport& transport;

public:
  //------------------------------------------------------------------------
  // Constructor - attach transport
  Client(ClientTransport& _transport): transport(_transport) {}

  //------------------------------------------------------------------------
  // Send a message - never blocks, but can fail if the queue is full
  // Whether message queued
  bool send(Message& msg);

  //------------------------------------------------------------------------
  // Receive a message - never blocks, returns whether one was received
  bool poll(Message& msg);

  //------------------------------------------------------------------------
  // Receive a message - blocks waiting for one to arrive
  // Returns whether one was read - will only return false if something fails
  bool wait(Message& msg);

  //------------------------------------------------------------------------
  // Send a message and get a response (blocking)
  // Returns whether successful, fills in response if so
  bool request(Message& req, Message& response);

  //------------------------------------------------------------------------
  // Send a message and confirm receipt or error
  // Returns whether successful.  Handles errors itself
  bool request(Message& req);

  //------------------------------------------------------------------------
  // Subscribe for messages of a given subject - expressed as a pattern match
  // e.g. client.subscribe("info.*");
  // Returns whether successful
  bool subscribe(const string& subject);

  //------------------------------------------------------------------------
  // Unsubscribe for messages of a given subject 
  // Subject is a pattern - can use more general pattern to unsubscribe
  // more specific ones
  // e.g. client.unsubscribe("*");
  // Returns whether successful
  bool unsubscribe(const string& subject);
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLMESH_CLIENT_H



