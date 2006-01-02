//==========================================================================
// ObTools::XMLMesh:Server: otmp-server.cc
//
// Implementation of OTMP server service for XMLMesh
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "server.h"
#include "ot-xmlmesh-otmp.h"
#include "ot-log.h"

#include <unistd.h>
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
  int port;

  OTMP::Server otmp;
  OTMPServerThread  server_thread;
  OTMPMessageThread message_thread;
  OTMP::ClientMessageQueue receive_q;

public:
  //------------------------------------------------------------------------
  // Constructor - default to standard OTMP port
  OTMPServer(XML::Element& cfg);

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
OTMPServer::OTMPServer(XML::Element& cfg):
  Service(cfg),
  port(cfg.get_attr_int("port", OTMP::DEFAULT_PORT)),
  otmp(receive_q, port), 
  server_thread(otmp), 
  message_thread(*this)
{
  bool filtered = false;
  OBTOOLS_XML_FOREACH_CHILD_WITH_TAG(filter, cfg, "filter")
    string addr = filter["address"];
    Net::MaskedAddress ma(addr);

    log.summary << "  Connections allowed from " << ma << endl;
    otmp.allow(ma);
    filtered = true;
  OBTOOLS_XML_ENDFOR

  // Add localhost-only filter if none specified
  if (!filtered)
  {
    otmp.allow(Net::MaskedAddress("localhost"));
    log.summary << "  Default filtering:  localhost only\n";
  }

  server_thread.start();
  message_thread.start();

  log.summary << "OTMP Server '" << id << "' started on port " 
	      << port << endl;
}

//--------------------------------------------------------------------------
// OTMP Message dispatcher
// Fetch OTMP messages and send into the system
void OTMPServer::dispatch() 
{ 
  OTMP::ClientMessage otmp_msg = receive_q.wait();

  // Create our reference for the client. 
  ServiceClient client(this, otmp_msg.client);

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
  Log::Streams tlog;  // Local log - can be called in any worker thread

  if (!msg.reversing)
  {
    tlog.error << "OTMP Server received forward routing\n";
    return false;
  }

  // Pop off the port and host from the path
  int port = msg.path.popi();
  string hosts = msg.path.pop();
  
  if (!port || !hosts.size())
  {
    tlog.error << "OTMP Server received bogus reverse path\n";
    return false;
  }

  Net::IPAddress host(hosts);
  if (!host)
  {
    tlog.error << "OTMP Server can't lookup reverse path host: " 
	       << hosts << endl;
    return false;
  }

  Net::EndPoint client(host, port); 

  OBTOOLS_LOG_IF_DEBUG(tlog.debug << "OTMP Server: responding to " 
		       << client << endl;)

  OTMP::ClientMessage otmp_msg(client, msg.message.get_text());
  if (otmp.send(otmp_msg))
  {
    // Tell tracker it was forwarded
    msg.notify_forwarded();
  }
  else tlog.error << "OTMP Server can't send message\n";  

  return false;  // Nowhere else to go
}

//==========================================================================
//Message thread run
void OTMPMessageThread::run()
{
  for(;;) service.dispatch();
}

//==========================================================================
// Auto-register
OT_XMLMESH_REGISTER_SERVICE(OTMPServer, "otmp-server");

}} // namespaces




