//==========================================================================
// ObTools::XMLBus: client.cc
//
// Implementation of XMLBus client library
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmlbus.h"
#include "ot-log.h"

#include <unistd.h>
#include <netinet/in.h>
#include <sstream>

namespace ObTools { namespace XMLBus {

//------------------------------------------------------------------------
// Constructor
Client::Client(Net::IPAddress address, int port):
  OTMPClient(address, port)
{

}


//------------------------------------------------------------------------
// Send a message - never blocks, but can fail if the queue is full
// Whether message queued
bool Client::send(Message& msg)
{
  // Create content string
  ostringstream contents;
  contents << "<otmp version=\"1.0\">\n"
           << "  <message subject=\"" << msg.subject << "\""
           << " id=\"" << msg.id << "\">\n  "
           << msg.content
           << "  </message>\n"
           << "</otmp>\n";
           
  OTMPMessage otmp_msg(contents.str());
  return OTMPClient::send(otmp_msg);
}

//------------------------------------------------------------------------
// Receive a message - never blocks, returns whether one was received
bool Client::poll(Message& msg)
{
  // Check if there's anything there
  if (!OTMPClient::poll()) return false;

  // Get it
  return wait(msg);
}

//------------------------------------------------------------------------
// Receive a message - blocks waiting for one to arrive
// Returns whether one was read - will only return false if something fails
bool Client::wait(Message& msg)
{
  OTMPMessage otmp_msg;
  if (!OTMPClient::wait(otmp_msg)) return false;

  msg.id = "?";
  msg.subject = "?"; //!!!
  msg.content = otmp_msg.data;
  return true;
}


}} // namespaces




