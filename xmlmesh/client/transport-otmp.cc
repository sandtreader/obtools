//==========================================================================
// ObTools::XMLMesh:Client: transport-otmp.cc
//
// Implementation of OTMP client transport for XMLMesh
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xmlmesh-client-otmp.h"
#include "ot-log.h"

#include <unistd.h>
#include <sstream>

namespace ObTools { namespace XMLMesh {

//------------------------------------------------------------------------
// Send a message - can block if the queue is full
// Whether message queued
bool OTMPClientTransport::send(const string& data)
{
  OTMP::Message otmp_msg(data);
  otmp.send(otmp_msg);
  return true;
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

//==========================================================================
// MessageInterface class

//--------------------------------------------------------------------------
// Constructor
OTMPMessageInterface::OTMPMessageInterface(XML::Element& config,
					   ObTools::Message::Broker& broker): 
  client(0)
{
  Log::Streams log;
  XML::XPathProcessor xpath(config);

  // Set up mesh connection - note, no default here, so if not present,
  // we disable it
  string host = xpath.get_value("server/@host");
  if (host.empty()) return;

  int port = xpath.get_value_int("server/@port", OTMP::DEFAULT_PORT);

  Net::IPAddress addr(host);
  if (!addr)
  {
    log.error << "Can't resolve XMLMesh host: " << host << endl;
    return;
  }

  Net::EndPoint ep(addr, port);
  log.summary << "Connecting to XMLMesh at " << ep << endl;

  // Start mesh client
  client = new OTMPMultiClient(ep);

  // Register our transport into server message broker
  broker.add_transport(new MessageTransport(*client));
}

//--------------------------------------------------------------------------
// Destructor - destroys message interface
OTMPMessageInterface::~OTMPMessageInterface()
{
  if (client) delete client;
}



}} // namespaces




