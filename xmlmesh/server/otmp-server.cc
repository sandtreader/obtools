//==========================================================================
// ObTools::XMLMesh:Server: otmp-server.cc
//
// Implementation of OTMP server service for XMLMesh
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "otmp-server.h"
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

class OTMPServer;  //forward

//==========================================================================
// OTMP Message thread class
class OTMPMessageThread: public MT::Thread
{
  OTMPServer& service;
  void run();

public:
  OTMPMessageThread(OTMPServer& _service):
    service(_service) {}
};

//==========================================================================
// OTMP Server Service
class OTMPServer: public Service
{
private:
  OTMP::Server otmp;
  OTMPServerThread  server_thread;
  OTMPMessageThread message_thread;
  OTMP::ClientMessageQueue receive_q;

public:
  //------------------------------------------------------------------------
  // Constructor - default to standard OTMP port
  OTMPServer(Server& server, XML::Element& cfg);

  //--------------------------------------------------------------------------
  // OTMP Message dispatcher
  // Fetch OTMP messages and send them up as internal messages
  void dispatch();

  //------------------------------------------------------------------------
  // Implementation of Service virtual interface - q.v. server.h
  bool handle(RoutingMessage& msg);
};

//------------------------------------------------------------------------
// Constructor
OTMPServer::OTMPServer(Server& server, XML::Element& cfg):
  Service(server, cfg),
  otmp(receive_q, cfg.get_attr_int("port", OTMP::DEFAULT_PORT)), 
  server_thread(otmp), 
  message_thread(*this)
{
  server_thread.start();
  message_thread.start();

  Log::Summary << "OTMP Server started\n";
}

//--------------------------------------------------------------------------
// OTMP Message dispatcher
// Fetch OTMP messages and send into the system
void OTMPServer::dispatch() 
{ 
  OTMP::ClientMessage otmp_msg = receive_q.wait();

  // Create our reference for the client. 
  Client client(this, otmp_msg.client);

  switch (otmp_msg.action)
  {
    case OTMP::ClientMessage::STARTED:
    {
      // Tell all services a new client has started
      server.signal_services(Service::CLIENT_STARTED, client);
      break;
    }

    case OTMP::ClientMessage::MESSAGE:
    {
      // Convert to routing message
      Message msg(otmp_msg.msg.data);
      RoutingMessage rmsg(client, msg);

      // Push our client info onto the path
      rmsg.path.push(otmp_msg.client.host.get_dotted_quad());
      rmsg.path.push(otmp_msg.client.port);

      // Send it into the system
      originate(rmsg);
      break;
    }

    case OTMP::ClientMessage::FINISHED:
    {
      // Tell all services a client has finished
      server.signal_services(Service::CLIENT_FINISHED, client);
      break;
    }
  }
}

//------------------------------------------------------------------------
// Implementation of Service virtual interface - q.v. server.h
// Note this only gets called for reversing messages coming back out
bool OTMPServer::handle(RoutingMessage& msg)
{
  if (!msg.reversing)
  {
    Log::Error << "OTMP Server received forward routing\n";
    return false;
  }

  // Pop off the port and host from the path
  int port = msg.path.popi();
  string hosts = msg.path.pop();
  
  if (!port || !hosts.size())
  {
    Log::Error << "OTMP Server received bogus reverse path\n";
    return false;
  }

  Net::IPAddress host(hosts);
  if (!host)
  {
    Log::Error << "OTMP Server can't lookup reverse path host: " 
	       << hosts << endl;
    return false;
  }

  Net::EndPoint client(host, port); 

  OTMP::ClientMessage otmp_msg(client, msg.message.get_text());
  if (!otmp.send(otmp_msg))
    Log::Error << "OTMP Server can't send message\n";  // Tell tracker!!!

  return false;  // Nowhere else to go
}

//==========================================================================
//Message thread run
void OTMPMessageThread::run()
{
  for(;;) service.dispatch();
}

//==========================================================================
// OTMP Server Factory

//------------------------------------------------------------------------
//Singleton instance
OTMPServerFactory OTMPServerFactory::instance;

//------------------------------------------------------------------------
//Create method
Service *OTMPServerFactory::create(Server& server, XML::Element& xml)
{
  return new OTMPServer(server, xml);
}

//------------------------------------------------------------------------
//Registration method
void OTMPServerFactory::register_into(Server& server)
{
  server.register_service("otmp-server", &instance);
}

}} // namespaces




