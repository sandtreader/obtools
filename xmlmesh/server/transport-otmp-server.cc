//==========================================================================
// ObTools::XMLMesh:Server: transport-otmp-server.cc
//
// Implementation of OTMP server transport for XMLMesh
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "transport-otmp-server.h"
#include "ot-xmlmesh-otmp.h"
#include "ot-log.h"

#include <unistd.h>
#include <netinet/in.h>
#include <sstream>

namespace ObTools { namespace XMLMesh {

//==========================================================================
// OTMP Server thread class
class OTMPServerThread: public MT::Thread
{
  OTMP::Server& server;
  void run()
  {
    server.run();
  }

public:
  OTMPServerThread(OTMP::Server &s): server(s) { }
};

class OTMPServerTransport;  //forward

//==========================================================================
// OTMP Message thread class
class OTMPMessageThread: public MT::Thread
{
  OTMPServerTransport& transport;
  void run();

public:
  OTMPMessageThread(OTMPServerTransport& _transport):
    transport(_transport) {}
};

//==========================================================================
// OTMP Server Transport 
class OTMPServerTransport: public Transport
{
private:
  OTMP::Server otmp;
  OTMPServerThread  server_thread;
  OTMPMessageThread message_thread;
  OTMP::ClientMessageQueue receive_q;

public:
  //------------------------------------------------------------------------
  // Constructor - default to standard OTMP port
  OTMPServerTransport(int port=0);

  //--------------------------------------------------------------------------
  // OTMP Message dispatcher
  // Fetch OTMP messages and send them up as internal messages
  void dispatch();

  //------------------------------------------------------------------------
  // Implementation of Transport virtual interface - q.v. server.h
  bool send(const Net::EndPoint& client, const string& data);
};

//------------------------------------------------------------------------
// Constructor
OTMPServerTransport::OTMPServerTransport(int port):
  otmp(receive_q, port), server_thread(otmp), 
  message_thread(*this)
{
  server_thread.start();
  message_thread.start();
}

//--------------------------------------------------------------------------
// OTMP Message dispatcher
// Fetch OTMP messages and send them up as internal messages
void OTMPServerTransport::dispatch() 
{ 
  if (incoming_q)
  {
    OTMP::ClientMessage otmp_msg = receive_q.wait();

    // Convert to generic message
    IncomingMessage imsg(otmp_msg.client, Message(otmp_msg.msg.data));
    incoming_q->send(imsg);
  }
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

//==========================================================================
//Message thread run
void OTMPMessageThread::run()
{
  for(;;) transport.dispatch();
}

//==========================================================================
// OTMP Server Transport Factory

//------------------------------------------------------------------------
//Singleton instance
OTMPServerTransportFactory OTMPServerTransportFactory::instance;

//------------------------------------------------------------------------
//Create method
Transport *OTMPServerTransportFactory::create(XML::Element& xml)
{
  int port = xml.get_attr_int("port", OTMP::DEFAULT_PORT);
  return new OTMPServerTransport(port);
}

//------------------------------------------------------------------------
//Registration method
void OTMPServerTransportFactory::register_into(Server& server)
{
  server.register_transport("otmp-server", &instance);
}

}} // namespaces




