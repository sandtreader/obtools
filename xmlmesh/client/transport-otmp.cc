//==========================================================================
// ObTools::XMLBus:OTMP: client_transport.cc
//
// Implementation of OTMP client transport for XMLBus
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmlbus-client-otmp.h"
#include "ot-log.h"

#include <unistd.h>
#include <netinet/in.h>
#include <sstream>

namespace ObTools { namespace XMLBus {

//------------------------------------------------------------------------
// Send a message - never blocks, but can fail if the queue is full
// Whether message queued
bool OTMPClientTransport::send(const string& data)
{
  OTMP::Message otmp_msg(data);
  return otmp.send(otmp_msg);
}

//------------------------------------------------------------------------
// Check for message available
bool OTMPClientTransport::poll()
{
  return otmp.poll();
}

//------------------------------------------------------------------------
// Receive a message - blocks waiting for one to arrive
// Returns whether one was read - will only return false if something fails
bool OTMPClientTransport::wait(string &data)
{
  OTMP::Message otmp_msg;
  if (!otmp.wait(otmp_msg)) return false;
  data = otmp_msg.data;
  return true;
}


}} // namespaces




