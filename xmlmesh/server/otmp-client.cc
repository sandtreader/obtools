//==========================================================================
// ObTools::XMLMesh:Server: otmp-client.cc
//
// Implementation of OTMP client service for XMLMesh
//
// Copyright (c) 2003-2004 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "server.h"
#include "ot-xmlmesh-client-otmp.h"
#include "ot-log.h"

#include <unistd.h>
#include <netinet/in.h>
#include <sstream>

namespace ObTools { namespace XMLMesh {

//==========================================================================
// OTMP Client thread class
class OTMPClientService; //forward

class OTMPClientThread: public MT::Thread
{
  OTMPClientService& service;
  void run();

public:
  OTMPClientThread(OTMPClientService& _service):
    service(_service) {}
};

//==========================================================================
// OTMP Client Service
class OTMPClientService: public Service
{
private:
  Net::EndPoint host;
  OTMPClient client;
  OTMPClientThread  client_thread;

public:
  //------------------------------------------------------------------------
  // Constructor 
  OTMPClientService(XML::Element& cfg);

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
OTMPClientService::OTMPClientService(XML::Element& cfg):
  Service(cfg),
  host(Net::IPAddress(cfg["server"]),
       cfg.get_attr_int("port", OTMP::DEFAULT_PORT)),
  client(host),
  client_thread(*this)
{
  client_thread.start();

  log.summary << "OTMP Client '" << id << "' to " << host << " started\n";

  OBTOOLS_XML_FOREACH_CHILD_WITH_TAG(sube, cfg, "subscription")
    string subject = sube["subject"];
    if (client.subscribe(subject))
      log.summary << "  Subscribed to " << subject << " at " << host << endl;
    else
      log.error << "OTMP Client to " << host << " can't subscribe to " 
		<< subject << endl;
  OBTOOLS_XML_ENDFOR
}

//--------------------------------------------------------------------------
// OTMP Message dispatcher
// Fetch OTMP messages and send into the system
void OTMPClientService::dispatch() 
{ 
  Message msg;

  if (client.wait(msg))
  {
    // Create our reference for the 'client' - the host at the other end
    ServiceClient sclient(this, host);

    // Convert to routing message
    RoutingMessage rmsg(sclient, msg);

    // Send it into the system
    originate(rmsg);
  }
  else
  {
    Log::Stream error_log(Log::logger, Log::LEVEL_ERROR);
    error_log << "OTMP Client connection restarted\n";
  }
}

//------------------------------------------------------------------------
// Implementation of Service virtual interface - q.v. server.h
bool OTMPClientService::handle(RoutingMessage& msg)
{
  if (!client.send(msg.message))
  {
    Log::Stream error_log(Log::logger, Log::LEVEL_ERROR);
    error_log << "OTMP Client can't send message\n";  // Tell tracker!!!
  }

  return false;  // Nowhere else to go
}

//==========================================================================
//client thread run
void OTMPClientThread::run()
{
  for(;;) service.dispatch();
}

//==========================================================================
// Auto-register
OT_XMLMESH_REGISTER_SERVICE(OTMPClientService, "otmp-client");

}} // namespaces




