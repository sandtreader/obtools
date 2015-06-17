//==========================================================================
// ObTools::XMLMesh:Server: http-server.cc
//
// Implementation of HTTP server service for XMLMesh
//
// Copyright (c) 2015 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "server.h"
#include "ot-log.h"
#include "ot-web.h"

#include <unistd.h>
#include <sstream>

namespace
{
  const std::string& HTTP_SERVER_VERSION = "ObTools XMLMesh HTTP Server";
  const int DEFAULT_PORT        = 29180;
  const int DEFAULT_BACKLOG     = 5;
  const int DEFAULT_MAX_THREADS = 25;
  const int DEFAULT_MIN_THREADS = 1;
  const int DEFAULT_TIMEOUT     = 0;

  // Default subscription timeout is set to be longer than the client poll
  // timeout, so that new polls refresh it, otherwise a subscription can finish
  // while a poll is still waiting
  const int DEFAULT_SUBSCRIPTION_TIMEOUT = 90;
  const int DEFAULT_POLL_TIMEOUT = 60;
}

namespace ObTools { namespace XMLMesh {

class HTTPServerService; // forward

//==========================================================================
// HTTP Server - accepts one of three POST requests:
//   /send - sends a one-way message
//     Request body is an XMLMesh SOAP message
//   /request - sends a request and gets a response
//     Request body is an XMLMesh SOAP message
//     Response body is XMLMesh SOAP message
//   /subscribe/<id> - sends a subscription request which can then be polled
//     with /poll with the same <id>
//
// ... and one GET:
//   /poll/<id> - polls for subscribed messages
//     Response body is XMLMesh SOAP message
class HTTPServer: public Web::HTTPServer
{
  HTTPServerService& service;

  // Implementation of HTTPServer downcalls
  bool handle_request(const Web::HTTPMessage& request,
                      Web::HTTPMessage& response,
                      const SSL::ClientDetails& client,
                      SSL::TCPSocket& socket,
                      Net::TCPStream& stream);

public:
  //--------------------------------------------------------------------------
  // Constructor to bind to any interface (basic TCP)
  // See ObTools::Net::TCPServer for details of threadpool management
  HTTPServer(HTTPServerService& _service, int port, int backlog,
	     int min_spare, int max_threads, int timeout):
    Web::HTTPServer(port, HTTP_SERVER_VERSION, backlog, min_spare,
                    max_threads, timeout),
    service(_service)
  {
    // Enable CORS, any origin
    set_cors_origin();
  }
};

//==========================================================================
// HTTP Server Service
class HTTPServerService: public Service
{
private:
  const int port;
  const int backlog;
  const int min_spare_threads;
  const int max_threads;
  const int timeout;

  HTTPServer http_server;
  Net::TCPServerThread http_server_thread;

  // Map of active requests/subscriptions based on source path, containing
  // a message queue for that client
  typedef MT::MQueue<string> ResponseQueue;
  typedef Gen::SharedPointer<ResponseQueue> ResponseQueuePtr;

  class ClientRequestMap:
    public Cache::UseTimeoutCache<string, ResponseQueuePtr>
  {
    HTTPServerService& service;

    // Implementation of delete_allowed, called when an item is about to
    // get aged out
    bool delete_allowed(const string& path, ResponseQueuePtr&)
    {
      service.client_request_timeout(path);
      return true;
    }

  public:
    ClientRequestMap(HTTPServerService& _service, int timeout):
      ObTools::Cache::UseTimeoutCache<string, ResponseQueuePtr>(timeout),
      service(_service) {}
  };

  int client_request_map_timeout;
  ClientRequestMap client_request_map;

  // Set of currently active polls, by message path, pointing to client request
  // for that path
  class ActivePollerMap:
    public Cache::AgeTimeoutCache<string, ResponseQueuePtr>
  {
    HTTPServerService& service;

    // Implementation of delete_allowed, called when an item is about to
    // get aged out
    bool delete_allowed(const string& path, ResponseQueuePtr& response_queue)
    {
      service.poller_timeout(path, response_queue);
      return true;
    }

  public:
    ActivePollerMap(HTTPServerService& _service, int timeout):
      ObTools::Cache::AgeTimeoutCache<string, ResponseQueuePtr>(timeout),
      service(_service) {}
  };

  int active_poller_map_timeout;
  ActivePollerMap active_poller_map;

public:
  //------------------------------------------------------------------------
  // Constructor - default to standard HTTP port
  HTTPServerService(const XML::Element& cfg);

  //------------------------------------------------------------------------
  // Check the service is happy
  bool started();

  //------------------------------------------------------------------------
  // Handle an incoming message POST request
  bool handle_request(const Web::HTTPMessage& request,
                      Web::HTTPMessage& response,
                      const string& path,
                      bool rsvp=false,
                      bool is_subscribe=false);

  //------------------------------------------------------------------------
  // Handle a poll GET request
  bool handle_poll(Web::HTTPMessage& response, const string& path="");

  //------------------------------------------------------------------------
  // Implementation of Service virtual interface - q.v. server.h
  bool handle(RoutingMessage& msg);
  void tick();

  //------------------------------------------------------------------------
  // Callback from client_request_map when a subscription request is timed out
  void client_request_timeout(const string& path);

  //------------------------------------------------------------------------
  // Callback from active_poller_map when a poller is timed out
  void poller_timeout(const string& path, ResponseQueuePtr& response_queue);
};

//==========================================================================
// HTTP Server implementation

//--------------------------------------------------------------------------
// Handle HTTP request (downcall from HTTP::Server)
bool HTTPServer::handle_request(const Web::HTTPMessage& request,
                                Web::HTTPMessage& response,
                                const SSL::ClientDetails& client,
                                SSL::TCPSocket& /*socket*/,
                                Net::TCPStream& /*stream*/)
{
  Log::Streams log;
  log.summary << "HTTP request from " << client << endl;

  // Split on / to get optional ID
  // bits[0] will be empty because of leading /
  const string& path = request.url.get_path();
  vector<string> bits = Text::split(path, '/');
  if (bits.size() < 2) return error(response, 403, "Forbidden");
  const string& command = bits[1];

  MessagePath mpath;
  if (bits.size() > 2)
  {
    // If ID is specified, use alone as path so it can be valid across
    // different connections
    mpath.push(bits[2]);
  }
  else
  {
    // Push the source address onto the path - valid only for this
    // connection
    mpath.push(client.address.host.get_dotted_quad());
    mpath.push(client.address.port);
  }
  const string& mpaths = mpath.to_string();

  // Split on command
  if (command == "send")
  {
    return service.handle_request(request, response, mpaths);
  }
  else if (command == "request")
  {
    return service.handle_request(request, response, mpaths, true);
  }
  else if (command == "subscribe")
  {
    return service.handle_request(request, response, mpaths, true, true);
  }
  else if (command == "poll")
  {
    if (service.handle_poll(response, mpaths)) return true;
    return error(response, 404, "No such subscription ID");
  }
  else
  {
    return error(response, 404, "Not found");
  }
}

//==========================================================================
// HTTP Server Service implementation

//------------------------------------------------------------------------
// Constructor
HTTPServerService::HTTPServerService(const XML::Element& cfg):
  Service(cfg),
  port(cfg.get_attr_int("port", DEFAULT_PORT)),
  backlog(cfg.get_attr_int("backlog", DEFAULT_BACKLOG)),
  min_spare_threads(cfg.get_attr_int("min-spare", DEFAULT_MIN_THREADS)),
  max_threads(cfg.get_attr_int("max-threads", DEFAULT_MAX_THREADS)),
  timeout(cfg.get_attr_int("timeout", DEFAULT_TIMEOUT)),
  http_server(*this, port, backlog, min_spare_threads, max_threads, timeout),
  http_server_thread(http_server),
  client_request_map_timeout(cfg.get_child("subscription")
                             .get_attr_int("timeout", DEFAULT_SUBSCRIPTION_TIMEOUT)),
  client_request_map(*this, client_request_map_timeout),
  active_poller_map_timeout(cfg.get_child("poll")
                            .get_attr_int("timeout", DEFAULT_POLL_TIMEOUT)),
  active_poller_map(*this, active_poller_map_timeout)
{
  Log::Streams log;
  log.summary << "HTTP server on port " << port << endl;
  log.detail << "Listen backlog " << backlog << endl;
  log.detail << "Minimum spare threads: " << min_spare_threads << endl;
  log.summary << "Maximum threads: " << max_threads << endl;
  if (timeout)
    log.summary << "Connection timeout: " << timeout << endl;

  log.summary << "Subscription timeout: " << client_request_map_timeout << endl;
  log.summary << "Poll timeout: " << active_poller_map_timeout << endl;
}

//------------------------------------------------------------------------
// Check the service is happy
// Override to close down startup if initialisation failed
bool HTTPServerService::started()
{
  if (!!http_server) return true;
  Log::Error log;
  log << "HTTP server failed to start\n";
  return false;
}

//------------------------------------------------------------------------
// Handle an incoming message POST request
bool HTTPServerService::handle_request(const Web::HTTPMessage& request,
                                       Web::HTTPMessage& response,
                                       const string& path,
                                       bool rsvp, bool is_subscribe)
{
  // Create mesh message from the body
  Message msg(request.body);
  RoutingMessage rmsg(msg);
  rmsg.path = path;

  // If subscription, announce our (persistent) presence
  if (is_subscribe)
  {
    RoutingMessage rmsg(RoutingMessage::CONNECTION, path);
    originate(rmsg);
  }

  // If RSVP, register into a map based on our source path as above
  ResponseQueuePtr response_queue;
  if (rsvp)
  {
    response_queue.reset(new ResponseQueue());
    client_request_map.add(path, response_queue);
  }

  // Send it into the system
  originate(rmsg);

  // If response required, wait for first message from queue
  if (rsvp)
  {
    response.body = response_queue->wait();

    // Leave request record in place if subscribed
    if (!is_subscribe) client_request_map.remove(path);
  }

  return true;
}

//------------------------------------------------------------------------
// Handle a poll GET request
bool HTTPServerService::handle_poll(Web::HTTPMessage& response,
                                    const string& path)
{
  // Keep it alive
  client_request_map.touch(path);

  // Look up the client request record and get its queue
  ResponseQueuePtr response_queue;
  {
    MT::RWReadLock lock(client_request_map.mutex);
    if (!client_request_map.lookup(path, response_queue)) return false;
  }

  // Add to active pollers
  active_poller_map.add(path, response_queue);

  // Wait for the response
  response.body = response_queue->wait();

  // Remove from active pollers again
  active_poller_map.remove(path);

  return true;
}

//------------------------------------------------------------------------
// Implementation of Service virtual interface - q.v. server.h
// Note this only gets called for reversing messages coming back out
bool HTTPServerService::handle(RoutingMessage& msg)
{
  Log::Streams log;

  switch (msg.type)
  {
    case RoutingMessage::MESSAGE:
    {
      if (!msg.reversing)
      {
        log.error << "HTTP Server received forward routing\n";
        return false;
      }

      string path = msg.path.to_string();  // The whole thing, whatever is left
      OBTOOLS_LOG_IF_DEBUG(log.debug << "HTTP Server: responding to "
                           << path << endl;)

      // Post response to worker thread that is waiting for it
      MT::RWReadLock lock(client_request_map.mutex);
      ResponseQueuePtr response_queue;
      if (client_request_map.lookup(path, response_queue))
        response_queue->send(msg.message.get_text());
      else
        log.error << "Orphan reverse message received to " << path << endl;
    }
    break;

    default:;
  }

  return false;  // Nowhere else to go
}

//------------------------------------------------------------------------
// Tick function
void HTTPServerService::tick()
{
  client_request_map.tidy();
  active_poller_map.tidy();
}

//------------------------------------------------------------------------
// Callback from client_request_map when an item is timed out
void HTTPServerService::client_request_timeout(const string& path)
{
  Log::Detail log;
  log << "HTTP server subscription timed out on " << path << endl;

  // Send DISCONNECTION routing message with this path
  RoutingMessage rmsg(RoutingMessage::DISCONNECTION, path);
  originate(rmsg);
}

//------------------------------------------------------------------------
// Callback from active_poller_map when a poller is timed out
void HTTPServerService::poller_timeout(const string& path,
                                       ResponseQueuePtr& response_queue)
{
  Log::Detail log;
  log << "Poller timed out on " << path << endl;

  // Keep any subscription alive for another full timeout period until the next
  // poll
  client_request_map.touch(path);

  // Send an empty message on the response queue to unblock it
  response_queue->send(string());
}

//==========================================================================
// Auto-register
OT_XMLMESH_REGISTER_SERVICE(HTTPServerService, "http-server");

}} // namespaces




