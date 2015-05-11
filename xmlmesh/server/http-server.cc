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
// HTTP Server - accepts one of two POST requests:
//   /send - sends a one-way message
//     Request body is an XMLMesh SOAP message
//   /request - sends a request and gets a response
//     Request body is an XMLMesh SOAP message
//     Response body is XMLMesh SOAP message
//   /subscribe - subscribes for messages
//     Request body is an XMLMesh subscription request
//     Response is potentially multiple SOAP messages in chunked encoding
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

  struct ClientRequest
  {
    string *response;
    MT::Condition *condition;
    ClientRequest(): response(0), condition(0) {}
    ClientRequest(string* _response, MT::Condition *_condition):
      response(_response), condition(_condition) {}
  };

  MT::RWMutex client_request_map_mutex;
  typedef map<Net::EndPoint, ClientRequest> client_request_map_t;
  client_request_map_t client_request_map;

public:
  //------------------------------------------------------------------------
  // Constructor - default to standard HTTP port
  HTTPServerService(XML::Element& cfg);

  //------------------------------------------------------------------------
  // Check the service is happy
  bool started();

  bool handle_request(Web::HTTPMessage& request,
                      Web::HTTPMessage& response,
                      SSL::ClientDetails& client,
                      bool rsvp);

  //------------------------------------------------------------------------
  // Implementation of Service virtual interface - q.v. server.h
  bool handle(RoutingMessage& msg);
};

//--------------------------------------------------------------------------
bool HTTPServer::handle_request(Web::HTTPMessage& request,
                                Web::HTTPMessage& response,
                                SSL::ClientDetails& client,
                                SSL::TCPSocket& /*socket*/,
                                Net::TCPStream& /*stream*/)
{
  Log::Streams log;
  log.summary << "HTTP request from " << client << endl;

  // Check URL for /request /send or /subscribe
  bool rsvp = false;
  const string& path = request.url.get_path();
  if (path == "/subscribe")
  {
    rsvp = true;
  }
  else if (path == "/request")
  {
    rsvp = true;
  }
  else if (path != "/send")
  {
    return error(response, 404, "Not found");
  }

  return service.handle_request(request, response, client, rsvp);
}

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
  log.error << "HTTP server failed to start\n";
  return false;
}

//------------------------------------------------------------------------
// Handle an incoming HTTP request
bool HTTPServerService::handle_request(Web::HTTPMessage& request,
                                       Web::HTTPMessage& response,
                                       SSL::ClientDetails& client,
                                       bool rsvp)
{
  // Create our reference for the client
  ServiceClient sclient(this, client.address);

  // Create mesh message from the body
  Message msg(request.body);
  RoutingMessage rmsg(sclient, msg);

  // Push our client info onto the path
  rmsg.path.push(client.address.host.get_dotted_quad());
  rmsg.path.push(client.address.port);

  // If RSVP, register into an endpoint map
  MT::Condition response_available;
  if (rsvp)
  {
    MT::RWWriteLock lock(client_request_map_mutex);
    client_request_map[client.address] = ClientRequest(&response.body,
                                                       &response_available);
  }

  // Send it into the system
  originate(rmsg);

  // If response required, block on it
  if (rsvp)
  {
    response_available.wait();

    MT::RWWriteLock lock(client_request_map_mutex);
    client_request_map.erase(client.address);
  }

  return true;
}


//------------------------------------------------------------------------
// Implementation of Service virtual interface - q.v. server.h
// Note this only gets called for reversing messages coming back out
bool HTTPServerService::handle(RoutingMessage& msg)
{
  Log::Streams tlog;  // Local log - can be called in any worker thread

  if (!msg.reversing)
  {
    tlog.error << "HTTP Server received forward routing\n";
    return false;
  }

  // Pop off the port and host from the path
  int port = msg.path.popi();
  string hosts = msg.path.pop();

  if (!port || !hosts.size())
  {
    tlog.error << "HTTP Server received bogus reverse path\n";
    return false;
  }

  Net::IPAddress host(hosts);
  if (!host)
  {
    tlog.error << "HTTP Server can't lookup reverse path host: "
	       << hosts << endl;
    return false;
  }

  Net::EndPoint address(host, port);

  OBTOOLS_LOG_IF_DEBUG(tlog.debug << "HTTP Server: responding to "
		       << address << endl;)

  // Post response to worker thread that is waiting for it
  MT::RWReadLock lock(client_request_map_mutex);
  client_request_map_t::iterator p = client_request_map.find(address);
  if (p != client_request_map.end())
  {
    ClientRequest& cr = p->second;
    *cr.response = msg.message.get_text();
    cr.condition->signal();
  }

  return false;  // Nowhere else to go
}

//==========================================================================
// Auto-register
OT_XMLMESH_REGISTER_SERVICE(HTTPServerService, "http-server");

}} // namespaces




