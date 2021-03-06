//==========================================================================
// ObTools::XMLMesh:Server: otmp-server.cc
//
// Implementation of OTMP server service for XMLMesh
//
// Copyright (c) 2003-2015 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "server.h"
#include "ot-xmlmesh-otmp.h"
#include "ot-log.h"

#include <unistd.h>
#include <sstream>

#define DEFAULT_BACKLOG 5
#define DEFAULT_MAX_THREADS 25
#define DEFAULT_MIN_THREADS 1
#define DEFAULT_TIMEOUT 0

namespace ObTools { namespace XMLMesh {

//==========================================================================
// OTMP Server thread class
class OTMPServerThread: public MT::Thread
{
  OTMP::Server& server;
  void run()
  {
    server.run();

    Log::Error log;
    log << "OTMP server shut down";
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
  const int port;
  const int backlog;
  const int min_spare_threads;
  const int max_threads;
  const int timeout;

  OTMP::Server otmp;
  OTMPServerThread  server_thread;
  OTMPMessageThread message_thread;
  OTMP::ClientMessageQueue receive_q;

public:
  //------------------------------------------------------------------------
  // Constructor - default to standard OTMP port
  OTMPServer(const XML::Element& cfg);

  //------------------------------------------------------------------------
  // Check the service is happy
  bool started() const;

  //------------------------------------------------------------------------
  // OTMP Message dispatcher
  // Fetch OTMP messages and send them up as internal messages
  // Returns false when exiting
  bool dispatch();

  //------------------------------------------------------------------------
  // Implementation of Service virtual interface - q.v. server.h
  bool handle(RoutingMessage& msg);

  //--------------------------------------------------------------------------
  // Clean shutdown
  void shutdown();
};

//--------------------------------------------------------------------------
// Constructor
OTMPServer::OTMPServer(const XML::Element& cfg):
  Service(cfg),
  port(cfg.get_attr_int("port", OTMP::DEFAULT_PORT)),
  backlog(cfg.get_attr_int("backlog", DEFAULT_BACKLOG)),
  min_spare_threads(cfg.get_attr_int("min-spare", DEFAULT_MIN_THREADS)),
  max_threads(cfg.get_attr_int("max-threads", DEFAULT_MAX_THREADS)),
  timeout(cfg.get_attr_int("timeout", DEFAULT_TIMEOUT)),
  otmp(receive_q, port, backlog, min_spare_threads, max_threads, timeout),
  server_thread(otmp),
  message_thread(*this)
{
  Log::Streams log;
  bool filtered = false;
  log.summary << "OTMP server on port " << port << endl;
  log.detail << "Listen backlog " << backlog << endl;
  log.detail << "Minimum spare threads: " << min_spare_threads << endl;
  log.summary << "Maximum threads: " << max_threads << endl;
  if (timeout)
    log.summary << "Timeout: " << timeout << endl;

  for(XML::Element::const_iterator p(cfg.get_children("filter")); p; ++p)
  {
    const XML::Element& filter = *p;
    string addr = filter["address"];
    Net::MaskedAddress ma(addr);

    log.summary << "  Connections allowed from " << ma << endl;
    otmp.allow(ma);
    filtered = true;
  }

  // Add localhost-only filter if none specified
  if (!filtered)
  {
    otmp.allow(Net::MaskedAddress("localhost"));
    log.summary << "  Default filtering:  localhost only\n";
  }

  server_thread.start();
  message_thread.start();
}

//--------------------------------------------------------------------------
// Check the service is happy
// Override to close down startup if initialisation failed
bool OTMPServer::started() const
{
  return (!!otmp && !!server_thread);
}

//--------------------------------------------------------------------------
// OTMP Message dispatcher
// Fetch OTMP messages and send into the system
// Returns false when exiting
bool OTMPServer::dispatch()
{
  const OTMP::ClientMessage otmp_msg = receive_q.wait();

  // Create path for this client = endpoint
  MessagePath path;
  path.push(otmp_msg.client.address.host.get_dotted_quad());
  path.push(otmp_msg.client.address.port);

  switch (otmp_msg.action)
  {
    case OTMP::ClientMessage::STARTED:
    {
      // Send CONNECTION routing message with this path
      RoutingMessage rmsg(RoutingMessage::CONNECTION, path);
      originate(rmsg);
    }
    break;

    case OTMP::ClientMessage::MESSAGE_DATA:
    {
      // Convert to routing message
      Message msg(otmp_msg.msg.data);
      RoutingMessage rmsg(msg);
      rmsg.path = path; // Note not with constructor because this is forward

      // Send it into the system
      originate(rmsg);
    }
    break;

    case OTMP::ClientMessage::FINISHED:
    {
      // Send DISCONNECTION routing message with this path
      RoutingMessage rmsg(RoutingMessage::DISCONNECTION, path);
      originate(rmsg);
    }
    break;

    case OTMP::ClientMessage::SHUTDOWN:
      return false; // exit thread
  }

  return true;
}

//--------------------------------------------------------------------------
// Implementation of Service virtual interface - q.v. server.h
// Note this only gets called for reversing messages coming back out
bool OTMPServer::handle(RoutingMessage& msg)
{
  Log::Streams log;  // Local log - can be called in any worker thread

  switch (msg.type)
  {
    case RoutingMessage::MESSAGE:
    {
      if (!msg.reversing)
      {
        log.error << "OTMP Server received forward routing\n";
        return false;
      }

      // Pop off the port and host from the path
      const int port = msg.path.popi();
      const string hosts = msg.path.pop();

      if (!port || !hosts.size())
      {
        log.error << "OTMP Server received bogus reverse path\n";
        return false;
      }

      const Net::IPAddress host(hosts);
      if (!host)
      {
        log.error << "OTMP Server can't lookup reverse path host: "
                  << hosts << endl;
        return false;
      }

      const Net::EndPoint address(host, port);
      const SSL::ClientDetails client(address, "");

      OBTOOLS_LOG_IF_DEBUG(log.debug << "OTMP Server: responding to "
                           << client << endl;)

      OTMP::ClientMessage otmp_msg(client, msg.message.get_text());
      if (otmp.send(otmp_msg))
      {
        // Tell tracker it was forwarded
        msg.notify_forwarded();
      }
      else log.error << "OTMP Server can't send message\n";
    }
    break;

    default: break;
  }

  return false;  // Nowhere else to go
}

//--------------------------------------------------------------------------
// Clean shutdown
void OTMPServer::shutdown()
{
  otmp.shutdown();
}

//==========================================================================
// Message thread run
void OTMPMessageThread::run()
{
  while (service.dispatch())
    ;
}

//==========================================================================
// Auto-register
OT_XMLMESH_REGISTER_SERVICE(OTMPServer, "otmp-server");

}} // namespaces




