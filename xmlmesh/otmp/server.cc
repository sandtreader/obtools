//==========================================================================
// ObTools::XMLMesh:OTMP: server.cc
//
// Implementation of raw OTMP server 
//
// Copyright (c) 2007 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xmlmesh-otmp.h"
#include "ot-log.h"

#include <unistd.h>
#include <sstream>

namespace ObTools { namespace XMLMesh { namespace OTMP {

//------------------------------------------------------------------------
// Constructor
Server::Server(ClientMessageQueue& receive_queue,
	       int port, int backlog, 
	       int min_spare_threads, int max_threads, int timeout):
  Tube::Server((port?port:DEFAULT_PORT), "OTMP", backlog, 
	       min_spare_threads, max_threads, timeout),
  receive_q(receive_queue)
{

}

//------------------------------------------------------------------------
// Function to handle an incoming client message
// Whether connection should be allowed to continue
bool Server::handle_message(const Tube::ClientMessage& msg)
{
  // We need to cast it to our own type to satisfy typing - but we don't
  // add anything to it, so this is safe
  receive_q.send(static_cast<const ClientMessage&>(msg));
  return true;
}


}}} // namespaces




