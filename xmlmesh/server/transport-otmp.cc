//==========================================================================
// ObTools::XMLBus:Server: transport-otmp.cc
//
// Implementation of OTMP server transport for XMLBus
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "transport-otmp.h"
#include "ot-log.h"

#include <unistd.h>
#include <netinet/in.h>
#include <sstream>

namespace ObTools { namespace XMLBus {

//--------------------------------------------------------------------------
// Send handler thread class
// Pulls messages off the given queue and sends them to the given socket
class ReflectorThread: public MT::Thread
{
  OTMP::Server& server;
  MT::Queue<OTMP::ClientMessage>& receive_q;

  void run() 
  { 
    for(;;)
    {
      // Block for a message
      OTMP::ClientMessage msg = receive_q.wait();

      // Send it back
      server.send(msg);
    }
  }

public:
  ReflectorThread(OTMP::Server &s, 
		  MT::Queue<OTMP::ClientMessage>& q): 
    server(s), receive_q(q) { start(); }
};


//------------------------------------------------------------------------
// Constructor
OTMPServerTransport::OTMPServerTransport(int port):
  otmp(receive_q, port)
{


}

//------------------------------------------------------------------------
// Send a message - never blocks, but can fail if the queue is full
// Whether message queued
bool OTMPServerTransport::send(const Net::EndPoint& client,
			       const string& data)
{
  OTMP::ClientMessage otmp_msg(client, data);
  return otmp.send(otmp_msg);
}


}} // namespaces




