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

#define HTTP_SERVER_VERSION "ObTools XMLMesh HTTP Server"

#define DEFAULT_PORT 29180
#define DEFAULT_BACKLOG 5
#define DEFAULT_MAX_THREADS 25
#define DEFAULT_MIN_THREADS 1
#define DEFAULT_TIMEOUT 0

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
  bool handle_request(Web::HTTPMessage& request,
                      Web::HTTPMessage& response,
                      SSL::ClientDetails& client,
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
    service(_service) {}
};

//==========================================================================
// HTTP Server Service
class HTTPServerService: public Service
{
private:
  int port;
  int backlog;
  int min_spare_threads;
  int max_threads;
  int timeout;

  HTTPServer http_server;
  Net::TCPServerThread http_server_thread;

  // Map of active requests/subscriptions based on source path, containing
  // a message queue for that client
  struct ClientRequest
  {
    Gen::SharedPointer<MT::MQueue<string> > response_queue;
    ClientRequest() {}
  };

  MT::RWMutex client_request_map_mutex;
  typedef map<string, ClientRequest> client_request_map_t;
  client_request_map_t client_request_map;

public:
  //------------------------------------------------------------------------
  // Constructor - default to standard HTTP port
  HTTPServerService(XML::Element& cfg);

  //------------------------------------------------------------------------
  // Check the service is happy
  bool started();

  //------------------------------------------------------------------------
  // Handle an incoming message POST request
  bool handle_request(Web::HTTPMessage& request,
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
};

//==========================================================================
// HTTP Server implementation

//--------------------------------------------------------------------------
// Handle HTTP request (downcall from HTTP::Server)
bool HTTPServer::handle_request(Web::HTTPMessage& request,
                                Web::HTTPMessage& response,
                                SSL::ClientDetails& client,
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
  string command = bits[1];

  MessagePath mpath;
  if (bits.size() > 2)
  {
    // If ID is specified, use alone as path so it can be valid across
    // different connections
    mpath.push(bits[2]);
  }
  else
  {
    // Push the soruce address onto the path - valid only for this
    // connection
    mpath.push(client.address.host.get_dotted_quad());
    mpath.push(client.address.port);
  }
  string mpaths = mpath.to_string();

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
HTTPServerService::HTTPServerService(XML::Element& cfg):
  Service(cfg),
  port(cfg.get_attr_int("port", DEFAULT_PORT)),
  backlog(cfg.get_attr_int("backlog", DEFAULT_BACKLOG)),
  min_spare_threads(cfg.get_attr_int("min-spare", DEFAULT_MIN_THREADS)),
  max_threads(cfg.get_attr_int("max-threads", DEFAULT_MAX_THREADS)),
  timeout(cfg.get_attr_int("timeout", DEFAULT_TIMEOUT)),
  http_server(*this, port, backlog, min_spare_threads, max_threads, timeout),
  http_server_thread(http_server)
{
  Log::Streams log;
  log.summary << "HTTP server on port " << port << endl;
  log.detail << "Listen backlog " << backlog << endl;
  log.detail << "Minimum spare threads: " << min_spare_threads << endl;
  log.summary << "Maximum threads: " << max_threads << endl;
  if (timeout)
    log.summary << "Timeout: " << timeout << endl;
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
bool HTTPServerService::handle_request(Web::HTTPMessage& request,
                                       Web::HTTPMessage& response,
                                       const string& path,
                                       bool rsvp, bool is_subscribe)
{
  // Create mesh message from the body
  Message msg(request.body);
  RoutingMessage rmsg(msg);
  rmsg.path = path;

  // If RSVP, register into a map based on our source path as above
  Gen::SharedPointer<MT::MQueue<string> > response_queue;
  if (rsvp)
  {
    MT::RWWriteLock lock(client_request_map_mutex);
    response_queue.reset(new MT::MQueue<string>());
    client_request_map[path].response_queue = response_queue;
  }

  // Send it into the system
  originate(rmsg);

  // If response required, wait for first message from queue
  if (rsvp)
  {
    response.body = response_queue->wait();

    // Leave request record in place if subscribed
    if (!is_subscribe)
    {
      MT::RWWriteLock lock(client_request_map_mutex);
      client_request_map.erase(path);
    }
  }

  return true;
}

//------------------------------------------------------------------------
// Handle a poll GET request
bool HTTPServerService::handle_poll(Web::HTTPMessage& response,
                                    const string& path)
{
  Gen::SharedPointer<MT::MQueue<string> > response_queue;

  // Look up the client request record and get its queue
  {
    MT::RWReadLock lock(client_request_map_mutex);
    client_request_map_t::iterator p = client_request_map.find(path);
    if (p != client_request_map.end())
    {
      ClientRequest& cr = p->second;
      response_queue = cr.response_queue;
    }
  }

  if (!response_queue) return false;

  // Wait for the response
  response.body = response_queue->wait();

  // Erase record
  // !!! for now, it could be left lying around in a timeout cache
  // !!! for successive polls
  {
    MT::RWWriteLock lock(client_request_map_mutex);
    client_request_map.erase(path);
  }

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
      MT::RWReadLock lock(client_request_map_mutex);
      client_request_map_t::iterator p = client_request_map.find(path);
      if (p != client_request_map.end())
      {
        ClientRequest& cr = p->second;
        cr.response_queue->send(msg.message.get_text());
      }
    }
    break;

    default:;
  }

  return false;  // Nowhere else to go
}

//==========================================================================
// Auto-register
OT_XMLMESH_REGISTER_SERVICE(HTTPServerService, "http-server");

}} // namespaces




