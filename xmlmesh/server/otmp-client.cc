//==========================================================================
// ObTools::XMLMesh:Server: otmp-client.cc
//
// Implementation of OTMP client service for XMLMesh
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================


///
/// Use OTMP::Client - client thread just waits and then injects messages
/// received back.  Handler just posts direct to client
///
#include "server.h"
#include "ot-xmlmesh-otmp.h"
#include "ot-log.h"

#include <unistd.h>
#include <netinet/in.h>
#include <sstream>

namespace ObTools { namespace XMLMesh {

//==========================================================================
// OTMP Client thread class
class OTMPClient; //forward

class OTMPClientThread: public MT::Thread
{
  OTMPClient& service;
  void run();

public:
  OTMPClientThread(OTMPClient& _service):
    service(_service) {}
};

//==========================================================================
// OTMP Client Service
class OTMPClient: public Service
{
private:
  Net::EndPoint host;
  OTMP::Client otmp;
  OTMPClientThread  client_thread;

public:
  //------------------------------------------------------------------------
  // Constructor 
  OTMPClient(XML::Element& cfg);

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
OTMPClient::OTMPClient(XML::Element& cfg):
  Service(cfg),
  host(Net::IPAddress(cfg["server"]),
       cfg.get_attr_int("port", OTMP::DEFAULT_PORT)),
  otmp(host),
  client_thread(*this)
{
  client_thread.start();

  Log::Summary << "OTMP Client '" << id << "' to " << host << " started\n";
}

//--------------------------------------------------------------------------
// OTMP Message dispatcher
// Fetch OTMP messages and send into the system
void OTMPClient::dispatch() 
{ 
  OTMP::Message otmp_msg;

  if (otmp.wait(otmp_msg))
  {
    // Create our reference for the 'client' - the host at the other end
    Client client(this, host);

    // Convert to routing message
    Message msg(otmp_msg.data);
    RoutingMessage rmsg(client, msg);

    // Send it into the system
    originate(rmsg);
  }
  else Log::Error << "OTMP Client connection restarted\n";
}

//------------------------------------------------------------------------
// Implementation of Service virtual interface - q.v. server.h
bool OTMPClient::handle(RoutingMessage& msg)
{
  OTMP::Message otmp_msg(msg.message.get_text());
  if (!otmp.send(otmp_msg))
    Log::Error << "OTMP Client can't send message\n";  // Tell tracker!!!

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
OT_XMLMESH_REGISTER_SERVICE(OTMPClient, "otmp-client");

}} // namespaces




