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
#include <memory>

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
  //------------------------------------------------------------------------
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

  // Map of active requests based on source path, containing
  // a message queue for that client
  typedef MT::Queue<string> ResponseQueue;
  typedef shared_ptr<ResponseQueue> ResponseQueuePtr;

  class ClientRequestMap:
    public Cache::BasicCache<string, ResponseQueuePtr>
  {

  public:
    ClientRequestMap() {}
  };

  ClientRequestMap client_request_map;

  // Similar, for long-term polls (survives any individual poll retry)
  // Keyed by subscriber ID
  typedef MT::Queue<string> PollQueue;
  typedef shared_ptr<PollQueue> PollQueuePtr;

  class ClientPollMap:
    public Cache::UseTimeoutCache<string, PollQueuePtr>
  {
    HTTPServerService& service;

    // Implementation of prepare_to_die, called when an item is about to
    // get aged out
    bool prepare_to_die(const string& subscriber_id, PollQueuePtr&)
    {
      service.client_poll_timeout(subscriber_id);
      return true;
    }

  public:
    ClientPollMap(HTTPServerService& _service, int timeout):
      ObTools::Cache::UseTimeoutCache<string, PollQueuePtr>(timeout),
      service(_service) {}
  };

  int subscription_timeout;
  ClientPollMap client_poll_map;

  // Set of currently active polls, by subscriber ID, pointing to client poll
  // for that path
  class ActivePollerMap:
    public Cache::AgeTimeoutCache<string, PollQueuePtr>
  {
    HTTPServerService& service;

    // Implementation of prepare_to_die, called when an item is about to
    // get aged out
    bool prepare_to_die(const string& subscriber_id, PollQueuePtr& poll_queue)
    {
      service.poller_timeout(subscriber_id, poll_queue);
      return true;
    }

  public:
    ActivePollerMap(HTTPServerService& _service, int timeout):
      ObTools::Cache::AgeTimeoutCache<string, PollQueuePtr>(timeout),
      service(_service) {}
  };

  int active_poller_map_timeout;
  ActivePollerMap active_poller_map;
  list<string> timed_out_subscribers;

public:
  //------------------------------------------------------------------------
  // Constructor - default to standard HTTP port
  HTTPServerService(const XML::Element& cfg);

  //------------------------------------------------------------------------
  // Check the service is happy
  bool started() const;

  //------------------------------------------------------------------------
  // Handle an incoming message POST request
  void handle_request(const Web::HTTPMessage& request,
                      Web::HTTPMessage& response,
                      const string& path,
                      const string& subscriber_id="",
                      bool rsvp=false);

  //------------------------------------------------------------------------
  // Handle a poll GET request
  void handle_poll(Web::HTTPMessage& response, const string& path,
                   const string& subscriber_id);

  //------------------------------------------------------------------------
  // Implementation of Service virtual interface - q.v. server.h
  bool handle(RoutingMessage& msg);
  void tick();

  //------------------------------------------------------------------------
  // Callback from client_poll_map when a poll is timed out
  void client_poll_timeout(const string& subscriber_id);

  //------------------------------------------------------------------------
  // Callback from active_poller_map when a poller is timed out
  void poller_timeout(const string& subscriber_id,
                      PollQueuePtr& response_queue);
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

  // Get subscriber ID, valid across multiple connections
  string subscriber_id;
  if (bits.size() > 2) subscriber_id = bits[2];

  // Push the source address onto the path - valid only for this
  // connection
  MessagePath mpath;
  mpath.push(client.address.host.get_dotted_quad());
  mpath.push(client.address.port);
  const string& mpath_s = mpath.to_string();

  // Split on command
  if (command == "send")
  {
    service.handle_request(request, response, mpath_s);
  }
  else if (command == "request")
  {
    service.handle_request(request, response, mpath_s, "", true);
  }
  else if (command == "subscribe" || command == "unsubscribe")
  {
    // Subscriber ID is mandatory
    if (subscriber_id.empty())
      return error(response, 400, "Bad request - no subscriber ID");
    service.handle_request(request, response, mpath_s, subscriber_id, true);
  }
  else if (command == "poll")
  {
    if (subscriber_id.empty())
      return error(response, 400, "Bad request - no subscriber ID");
    service.handle_poll(response, mpath_s, subscriber_id);
  }
  else
  {
    return error(response, 404, "Not found");
  }

  return true;
}

//==========================================================================
// HTTP Server Service implementation

//--------------------------------------------------------------------------
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
  subscription_timeout(cfg.get_child("subscription")
                       .get_attr_int("timeout", DEFAULT_SUBSCRIPTION_TIMEOUT)),
  client_poll_map(*this, subscription_timeout),
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

  log.summary << "Subscription timeout: " << subscription_timeout << endl;
  log.summary << "Poll timeout: " << active_poller_map_timeout << endl;
}

//--------------------------------------------------------------------------
// Check the service is happy
// Override to close down startup if initialisation failed
bool HTTPServerService::started() const
{
  return !!http_server;
}

//--------------------------------------------------------------------------
// Handle an incoming message POST request
void HTTPServerService::handle_request(const Web::HTTPMessage& request,
                                       Web::HTTPMessage& response,
                                       const string& path,
                                       const string& subscriber_id,
                                       bool rsvp)
{
  // Create mesh message from the body
  Message msg(request.body);
  RoutingMessage rmsg(msg);
  rmsg.path = path;
  rmsg.subscriber_id = subscriber_id;

  // If RSVP, register into a map based on our source path as above
  // Note this has to be unique per connection
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
    client_request_map.remove(path);
  }
}

//--------------------------------------------------------------------------
// Handle a poll GET request
void HTTPServerService::handle_poll(Web::HTTPMessage& response,
                                    const string& path,
                                    const string& subscriber_id)
{
  // Keep it alive
  client_poll_map.touch(subscriber_id);

  // Look up the client request record and get its queue
  PollQueuePtr poll_queue;
  {
    MT::RWWriteLock lock(client_poll_map.mutex);
    if (!client_poll_map.lookup(subscriber_id, poll_queue))
    {
      Log::Detail log;
      log << "Creating new HTTP poller on " << path << " with ID "
          << subscriber_id << endl;

      // Create it
      poll_queue.reset(new PollQueue());
      client_poll_map.add(subscriber_id, poll_queue);

      // Announce connection
      RoutingMessage crmsg(RoutingMessage::CONNECTION, path, subscriber_id);
      originate(crmsg);
    }
  }

  // Add to active pollers
  active_poller_map.add(subscriber_id, poll_queue);

  // Wait for a message
  response.body = poll_queue->wait();

  // Remove from active pollers again
  active_poller_map.remove(subscriber_id);
}

//--------------------------------------------------------------------------
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

      // Post responses with 'ref' to response queue, the rest (subscribed
      // messages) to poll_queue
      if (msg.message.get_ref().empty())
      {
        log.detail << "HTTP Server: returning subscribed message "
                   << msg.message.get_subject()
                   << " to ID " << msg.subscriber_id << endl;

        MT::RWReadLock lock(client_poll_map.mutex);
        PollQueuePtr poll_queue;

        if (client_poll_map.lookup(msg.subscriber_id, poll_queue))
          poll_queue->send(msg.message.get_text());
        else
          log.error << "Orphan reverse message received for subscriber ID "
                    << msg.subscriber_id << endl;
      }
      else
      {
        string path = msg.path.to_string();
        OBTOOLS_LOG_IF_DEBUG(log.debug << "HTTP Server: responding to "
                             << path << endl;)

        MT::RWReadLock lock(client_request_map.mutex);
        ResponseQueuePtr response_queue;
        if (client_request_map.lookup(path, response_queue))
          response_queue->send(msg.message.get_text());
        else
          log.error << "Orphan response message received to " << path << endl;
      }
    }
    break;

    default:;
  }

  return false;  // Nowhere else to go
}

//--------------------------------------------------------------------------
// Tick function
void HTTPServerService::tick()
{
  client_request_map.tidy();
  active_poller_map.tidy();
  client_poll_map.tidy();

  // Send disconnection messages outside tidy() lock to avoid mutual
  // deadlock where we lock client_poll_map mutex and then Publisher mutex
  // to do the unsubscribe_all(), while an incoming message does the reverse
  // Note we are safe to do this without mutex on timed_out_subscribers
  // because this function (and its call to client_poll_map.tidy()) is
  // the only thing that ever touches it
  for(const auto& id: timed_out_subscribers)
  {
    // Send DISCONNECTION routing message with this ID, empty path
    MessagePath path;
    RoutingMessage rmsg(RoutingMessage::DISCONNECTION, path, id);
    originate(rmsg);
  }
  timed_out_subscribers.clear();
}

//--------------------------------------------------------------------------
// Callback from client_poll_map when an item is timed out
void HTTPServerService::client_poll_timeout(const string& subscriber_id)
{
  Log::Detail log;
  log << "HTTP server poll ID " << subscriber_id << " timed out\n";
  timed_out_subscribers.push_back(subscriber_id);
}

//--------------------------------------------------------------------------
// Callback from active_poller_map when a poller is timed out
void HTTPServerService::poller_timeout(const string& subscriber_id,
                                       PollQueuePtr& poll_queue)
{
  Log::Detail log;
  log << "Poller ID " << subscriber_id << " timed out\n";

  // Keep any subscription alive for another full timeout period until the next
  // poll
  client_poll_map.touch(subscriber_id);

  // Send an empty message on the poll queue to unblock it
  poll_queue->send(string());
}

//==========================================================================
// Auto-register
OT_XMLMESH_REGISTER_SERVICE(HTTPServerService, "http-server");

}} // namespaces




