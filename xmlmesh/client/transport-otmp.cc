//==========================================================================
// ObTools::XMLMesh:Client: transport-otmp.cc
//
// Implementation of OTMP client transport for XMLMesh
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-xmlmesh-client-otmp.h"
#include "ot-log.h"

#include <unistd.h>
#include <netinet/in.h>
#include <sstream>

namespace ObTools { namespace XMLMesh {

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
#if defined(_SINGLE)
  Log::Error << "Poll called on single-threaded OTMP transport\n";
  return false;
#else
  return otmp.poll();
#endif
}

//------------------------------------------------------------------------
// Receive a message - blocks waiting for one to arrive
// Returns false if the transport was restarted and subscriptions
// (and messages) may have been lost
bool OTMPClientTransport::wait(string &data)
{
  OTMP::Message otmp_msg;
  if (!otmp.wait(otmp_msg)) return false;
  data = otmp_msg.data;
  return true;
}


}} // namespaces




