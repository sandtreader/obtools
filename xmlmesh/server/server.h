//==========================================================================
// ObTools::XMLMesh:Server: server.h
//
// Internal definitions for XMLMesh Server
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_XMLMESH_SERVER_H
#define __OBTOOLS_XMLMESH_SERVER_H

#include <string>
#include "ot-net.h"
#include "ot-log.h"
#include "ot-mt.h"
#include "ot-cache.h"
#include "ot-xmlmesh.h"
#include "ot-init.h"

namespace ObTools { namespace XMLMesh {

//Make our lives easier without polluting anyone else
using namespace std;

class Service;  // Forward
class Server;     

//==========================================================================
// Signature of a client
struct ServiceClient
{
  Service *service;
  Net::EndPoint client;

  // Constructor
  ServiceClient(Service *_service, Net::EndPoint _client):
    service(_service), client(_client) {}

  // Comparators
  bool operator==(const ServiceClient& o)
  { return service==o.service && client==o.client; }
  bool operator!=(const ServiceClient& o) 
  { return service!=o.service || client!=o.client; }
};

//==========================================================================
// Path of a message through the system
class MessagePath
{
  deque<string> path;

 public:
  //------------------------------------------------------------------------
  // Constructors
  MessagePath() {}  // empty
  MessagePath(const string& s);  // From | delimited string (top last)

  //------------------------------------------------------------------------
  // Push a path level
  void push(const string& s) { path.push_back(s); }

  //------------------------------------------------------------------------
  // Push a path level integer 
  void push(int n);

  //------------------------------------------------------------------------
  // Pop a path level
  string pop() { string s=path.back(); path.pop_back(); return s; }

  //------------------------------------------------------------------------
  // Pop a path level integer
  int popi();

  //------------------------------------------------------------------------
  // Generate a | delimited string (top last)
  string to_string();
};

//==========================================================================
// Message to be routed through the system
struct RoutingMessage
{
  ServiceClient client;        // Original client received from
  Message message;             // The message
  bool reversing;              // Is this a response going back up the chain?

  MessagePath path;            // Path through the internal services
                               // - built up for requests, stripped back for
                               //   responses
 
  //------------------------------------------------------------------------
  // Constructor for inbound messages with empty path
  RoutingMessage(ServiceClient& _client, Message _message):
    client(_client), message(_message), reversing(false)
  {}

  //------------------------------------------------------------------------
  // Constructor for returned messages, with reverse path
  RoutingMessage(ServiceClient& _client, Message _message, MessagePath& _path):
    client(_client), message(_message), reversing(true), path(_path)
  {}
};

//==========================================================================
// Onward message route
struct MessageRoute
{
  string subject_pattern;
  Service& service;

  MessageRoute(const string& _pattern, Service& _service):
    subject_pattern(_pattern), service(_service) {}
};

//==========================================================================
// Service thread for multi-threaded services
class ServiceThread: public MT::PoolThread
{
public:
  Service *service;
  RoutingMessage *msg;
  
  ServiceThread(ObTools::MT::PoolReplacer<ServiceThread>& _rep):
    PoolThread(_rep) {}

  //------------------------------------------------------------------------
  // Run function - pass msg to service 
  void run();
};

//==========================================================================
// Service (abstract) - a message receiver and/or generator
class Service
{
private:
  friend class ServiceThread;

  list<MessageRoute> routes;    // List of routes for onward propagation
  bool multithreaded;
  MT::ThreadPool<ServiceThread> threads;

  void work(RoutingMessage& msg);
  bool forward(RoutingMessage& msg);
  bool reverse(RoutingMessage& msg);

protected:
  string id;
  Log::Streams log;             // Private log for foreground operations only

  //------------------------------------------------------------------------
  // Virtual message handler to be implemented by subclasses
  // Note:  This can get called in a worker thread!
  // Act on / modify message as required.  Forwarding is automatic.
  // Returns whether message should be forwarded
  virtual bool handle(RoutingMessage& msg) = 0;

  //------------------------------------------------------------------------
  // Originate a new message - handles deletion when finished with
  // Returns whether successful
  bool originate(RoutingMessage& msg);

  //------------------------------------------------------------------------
  // Return a message as a response to an existing one
  // Returns whether successul
  bool respond(Message& response, RoutingMessage& request);

  //------------------------------------------------------------------------
  // Return OK to an existing request
  // Returns whether successul
  bool respond(RoutingMessage& request);

  //------------------------------------------------------------------------
  // Return a fault to an existing request
  // Returns whether successul
  bool respond(RoutingMessage& request,
	       SOAP::Fault::Code code,
	       const string& reason);

public:
  //------------------------------------------------------------------------
  // Constructors
  Service(const string& _id, 
	  int _min_threads=1, int _max_threads=1): 
    multithreaded(_max_threads>1), 
    threads(_min_threads, _max_threads),
    id(_id)
  {}

  // From XML config
  Service(XML::Element& cfg):
    multithreaded(cfg.get_attr_int("minthreads", 1) > 1),
    threads(cfg.get_attr_int("minthreads", 1),
	    cfg.get_attr_int("maxthreads", 1)),
    id(cfg["id"])
  {}

  //------------------------------------------------------------------------
  // Add a new route on the given subject pattern
  void add_route(const string& subject, Service& service)
  { routes.push_back(MessageRoute(subject, service)); }

  //------------------------------------------------------------------------
  // Accept a message 
  // Performs local processing on messages (optionally in a worker thread)
  // and then forwards it to routes (also optionally in worker threads)
  void accept(RoutingMessage& msg);

  //------------------------------------------------------------------------
  // Signal various global events, independent of message routing
  // Does nothing by default, can be overridden
  enum Signal
  {
    CLIENT_STARTED,
    CLIENT_FINISHED
  };

  virtual void signal(Signal, ServiceClient&) {}

  //------------------------------------------------------------------------
  // Tick function - does nothing by default, can be overridden
  virtual void tick() {}

  //------------------------------------------------------------------------
  // Virtual destructor - does nothing here
  virtual ~Service() {}
};

//==========================================================================
// General XML Bus server using any number of transports
class Server
{
private:
  // List of active modules
  list<Service *> services;                 // List of active services
  map<string, Service *> service_ids;       // Map of service ids
  
  Log::Streams log;                         // Foreground logging

  // Internal functions
  bool create_service(XML::Element& xml); 
  bool create_route(XML::Element& xml); 

public:
  // Factories for use during configuration
  Init::Registry<Service> service_registry;

  //------------------------------------------------------------------------
  // Constructor 
  Server() {}

  //------------------------------------------------------------------------
  // Signal all services a global event (e.g. clients starting and finishing)
  void signal_services(Service::Signal sig, ServiceClient& client);

  //------------------------------------------------------------------------
  // Look up a service by id
  Service *lookup_service(const string& id);

  //------------------------------------------------------------------------
  // Load modules etc. from XML config
  void configure(XML::Configuration& config);

  //------------------------------------------------------------------------
  // Run method - never returns
  void run(); 

  //------------------------------------------------------------------------
  // Destructor
  ~Server();
};

// Global server instance
extern Server server;

// Macro for auto-registration of services into registry
#define OT_XMLMESH_REGISTER_SERVICE(_subclass, _name)  \
static Init::AutoRegister<Service, _subclass> \
  __ar(server.service_registry, _name);

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLMESH_SERVER_H



