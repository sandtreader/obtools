//==========================================================================
// ObTools::XMLBus: server.cc
//
// Implementation of raw OTMP server 
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmlbus-otmp.h"
#include "ot-log.h"

#include <unistd.h>
#include <netinet/in.h>
#include <sstream>

namespace ObTools { namespace XMLBus {

//------------------------------------------------------------------------
// TCPServer process method - called in worker thread to handle connection
void OTMPServer::process(Net::TCPSocket& s, 
			 Net::IPAddress client_address,
			 int client_port)
{
  Log::Summary << "OTMP(serv): Got connection from " << client_address 
	       << ":" << client_port << endl;

  try
  {
    // Just reflect everything
    string buf;
    while (s >> buf) s << buf;

    Log::Summary << "OTMP(serv): Connection from " << client_address 
		 << ":" << client_port << " ended\n";
  }
  catch (Net::SocketError se)
  {
    cerr << se << endl;
  }
} 


//------------------------------------------------------------------------
// Constructor
OTMPServer::OTMPServer(MT::Queue<OTMPMessage>& receive_queue,
		       int port, int backlog, 
		       int min_spare_threads, int max_threads):
  receive_q(receive_queue),
  TCPServer((port?port:OTMP_DEFAULT_PORT), backlog, 
	    min_spare_threads, max_threads)
{

}


//------------------------------------------------------------------------
// Send a message - never blocks, but can fail if the queue is full
// Whether message queued
bool OTMPServer::send(OTMPMessage& msg)
{
  send_q.send(msg);  // Never fails, will eat all memory first
  return true;  
}

}} // namespaces




