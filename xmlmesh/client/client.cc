//==========================================================================
// ObTools::XMLMesh: client.cc
//
// Implementation of XMLMesh client library
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmlmesh-client.h"
#include "ot-log.h"

#include <unistd.h>
#include <netinet/in.h>
#include <sstream>

namespace ObTools { namespace XMLMesh {

//------------------------------------------------------------------------
// Send a message - never blocks, but can fail if the queue is full
// Whether message queued
bool Client::send(Message& msg)
{
  string data = msg.get_text();  // Convert to <message> string
  return transport.send(data);
}

//------------------------------------------------------------------------
// Receive a message - never blocks, returns whether one was received
bool Client::poll(Message& msg)
{
  // Check if there's anything there
  if (!transport.poll()) return false;

  // Get it
  return wait(msg);
}

//------------------------------------------------------------------------
// Receive a message - blocks waiting for one to arrive
// Returns whether one was read - will only return false if something fails
bool Client::wait(Message& msg)
{
  string data;
  if (!transport.wait(data)) return false;

  msg = Message(data);  // Construct message from XML <message> data
  return true;
}


}} // namespaces




