//==========================================================================
// ObTools::XMLMesh:Server: otmp-client.cc
//
// Implementation of OTMP client service for XMLMesh
//
// Copyright (c) 2003-2015 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "server.h"
#include "ot-xmlmesh-client-otmp.h"
#include "ot-log.h"

#include <unistd.h>
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
  const Net::EndPoint host;
  list<string> subjects;
  unique_ptr<OTMPClient> client;
  OTMPClientThread  client_thread;
  bool running{true};

  void shutdown();

public:
  //------------------------------------------------------------------------
  // Constructor
  OTMPClientService(const XML::Element& cfg);

  //--------------------------------------------------------------------------
  // OTMP Subscriber
  // Subscribe all requested subjects
  // Note:  May block, so called at start of background thread
  void subscribe();

  //------------------------------------------------------------------------
  // OTMP Message dispatcher
  // Fetch OTMP messages and send them up as internal messages
  // Returns whether still running
  bool dispatch();

  //------------------------------------------------------------------------
  // Implementation of Service virtual interface - q.v. server.h
  bool handle(RoutingMessage& msg);
};

//--------------------------------------------------------------------------
// Constructor
OTMPClientService::OTMPClientService(const XML::Element& cfg):
  Service(cfg),
  host(Net::IPAddress(cfg["server"]),
       cfg.get_attr_int("port", OTMP::DEFAULT_PORT)),
  client_thread(*this)
{
  // Get subjects
  for(XML::Element::const_iterator p(cfg.get_children("subscription")); p; ++p)
  {
    const XML::Element& sube = *p;
    subjects.push_back(sube["subject"]);
  }

  // Start background thread
  client_thread.start();
}

//--------------------------------------------------------------------------
// OTMP Subscriber
// Subscribe all requested subjects
// Note:  May block, so called at start of background thread
void OTMPClientService::subscribe()
{
  Log::Streams log;

  // Connect client
  log.summary << "OTMP Client '" << id << "' connecting to " << host << endl;
  client.reset(new OTMPClient(host));

  // Subscribe
  for(const auto& subject: subjects)
  {
    if (client->subscribe(subject))
      log.summary << "  Subscribed to " << subject << " at " << host << endl;
    else
      log.error << "OTMP Client to " << host << " can't subscribe to "
                << subject << endl;
  }
}

//--------------------------------------------------------------------------
// OTMP Message dispatcher
// Fetch OTMP messages and send into the system
bool OTMPClientService::dispatch()
{
  Message msg;

  if (client->wait(msg))
  {
    // Convert to routing message
    RoutingMessage rmsg(msg);

    // Send it into the system
    originate(rmsg);
  }
  else
  {
    Log::Error log;
    log << "OTMP Client connection restarted\n";
  }

  return running;
}

//--------------------------------------------------------------------------
// Implementation of Service virtual interface - q.v. server.h
bool OTMPClientService::handle(RoutingMessage& msg)
{
  switch (msg.type)
  {
    case RoutingMessage::MESSAGE:
    {
      if (client->send(msg.message))
      {
        // Tell tracker the message has been forwarded, so it can call off
        // the dogs locally
        msg.notify_forwarded();
      }
      else
      {
        Log::Error log;
        log << "OTMP Client can't send message\n";
      }
    }
    break;

    default:;
  }

  return false;  // Nowhere else to go
}

//--------------------------------------------------------------------------
// Shut down
void OTMPClientService::shutdown()
{
  running = false;
  if (!!client) client->shutdown();
  client_thread.join();
  client.reset();
}

//==========================================================================
// client thread run
void OTMPClientThread::run()
{
  // Subscribe before running dispatcher to avoid OK results getting stolen
  service.subscribe();
  while (service.dispatch())
    ;
}

//==========================================================================
// Auto-register
OT_XMLMESH_REGISTER_SERVICE(OTMPClientService, "otmp-client");

}} // namespaces




