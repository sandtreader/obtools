//==========================================================================
// ObTools::XMLBus:Server server.cc
//
// Implementation of XMLBus server object
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "server.h"
#include "ot-log.h"

#include <unistd.h>
#include <netinet/in.h>
#include <sstream>

namespace ObTools { namespace XMLBus { 

//------------------------------------------------------------------------
// Constructor
Server::Server()
{
}

//------------------------------------------------------------------------
// Attach a new transport
void Server::attach_transport(ServerTransport *t)
{
  // Connect to our incoming queue
  t->attach_incoming(incoming_q);

  transports.push_back(t);
}

//------------------------------------------------------------------------
// Destructor
Server::~Server()
{
  for(list<ServerTransport *>::iterator p=transports.begin();
      p!=transports.end();
      p++)
    delete *p;
}

}} // namespaces




