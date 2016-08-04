//==========================================================================
// ObTools::XMLMesh:Server: server.h
//
// Internal definitions for XMLMesh Server
//
// Copyright (c) 2003-2015 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
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
  ServiceClient(Service *_service, const Net::EndPoint& _client):
    service(_service), client(_client) {}

  // Comparators
  bool operator==(const ServiceClient& o) const
  { return service==o.service && client==o.client; }
  bool operator!=(const ServiceClient& o) const
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
  // Check whether empty
  bool empty() const { return path.empty(); }

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
  string to_string() const;
};

//==========================================================================
// Abstract interface for message tracking

class RoutingMessage;  // forward

class MessageTracker
{
public:
  //------------------------------------------------------------------------
  // Attach a (new) copy of a message to the tracker
  virtual void attach(RoutingMessage *msg) = 0;

  //------------------------------------------------------------------------
  // Detach a copy of a message (before it dies)
  virtual void detach(RoutingMessage *msg) = 0;

  //------------------------------------------------------------------------
  // Notify tracker of forwarding of a message
  virtual void notify_forwarded(RoutingMessage *msg) = 0;

  //------------------------------------------------------------------------
  // Virtual destructor (does nothing here)
  virtual ~MessageTracker() {}
};

//==========================================================================
// Message to be routed through the system
class RoutingMessage
{
public:
  enum Type
  {
    CONNECTION,   // New connection from a given client path
    MESSAGE,      // An actual message
    DISCONNECTION // Disconnection from the given client path
  };
  Type type;
  Message message;             // The message
  bool reversing;              // Is this a response going back up the chain?

  MessagePath path;            // Path through the internal services
                               // - built up for requests, stripped back for
                               //   responses
  MessageTracker *tracker;     // Tracker following this message, or 0

  //------------------------------------------------------------------------
  // Constructor for inbound messages with empty path
  RoutingMessage(const Message& _message):
    type(MESSAGE), message(_message), reversing(false), tracker(0)
  {}

  //------------------------------------------------------------------------
  // Constructor for returned messages, with reverse path
  RoutingMessage(const Message& _message, const MessagePath& _path):
    type(MESSAGE), message(_message), reversing(true), path(_path), tracker(0)
  {}

  //------------------------------------------------------------------------
  // Constructor for informational types (CONNECTION, DISCONNECTION) with
  // no message or path (yet)
  RoutingMessage(Type _type, const MessagePath& _path):
    type(_type), reversing(false), path(_path), tracker(0)
  {}

  //------------------------------------------------------------------------
  // Copy constructor
  // Piecewise copy, except tells tracker that it was copied, too
  RoutingMessage(const RoutingMessage& orig):
    type(orig.type), message(orig.message), reversing(orig.reversing),
    path(orig.path), tracker(orig.tracker)
  {
    // Attach new copy to tracker, if any
    if (tracker) tracker->attach(this);
  }

  //------------------------------------------------------------------------
  // Attach tracker
  void track(MessageTracker *_tracker)
  {
    // Detach any old tracker
    untrack();
    tracker = _tracker;
    tracker->attach(this);
  }

  //------------------------------------------------------------------------
  // Detach tracker
  void untrack()
  {
    if (tracker)
    {
      tracker->detach(this);
      tracker = 0;
    }
  }

  //------------------------------------------------------------------------
  // Notify of forwarding
  void notify_forwarded()
  {
    // Tell tracker, if any
    if (tracker) tracker->notify_forwarded(this);
  }

  //------------------------------------------------------------------------
  // Destructor - tell tracker of our demise
  ~RoutingMessage()
  {
    if (tracker) tracker->detach(this);
  }
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
  Service *service;
  RoutingMessage *msg;

public:
  ServiceThread(ObTools::MT::PoolReplacer<ServiceThread>& _rep):
    PoolThread(_rep) {}

  //------------------------------------------------------------------------
  // Set up parameters for run
  void setup(Service *_s, RoutingMessage *_m)
  {
    service = _s;
    msg = _m;
  }

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
  bool respond(const Message& response, const RoutingMessage& request);

  //------------------------------------------------------------------------
  // Return OK to an existing request
  // Returns whether successul
  bool respond(const RoutingMessage& request);

  //------------------------------------------------------------------------
  // Return a fault to an existing request
  // Returns whether successul
  bool respond(const RoutingMessage& request,
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
  Service(const XML::Element& cfg):
    multithreaded(cfg.get_attr_int("minthreads", 1) > 1),
    threads(cfg.get_attr_int("minthreads", 1),
            cfg.get_attr_int("maxthreads", 1)),
    id(cfg["id"])
  {}

  //------------------------------------------------------------------------
  // Get ID
  const string& get_id() const { return id; }

  //------------------------------------------------------------------------
  // Check the service is happy
  // Override to close down startup if initialisation failed
  virtual bool started() const { return true; }

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

  // Internal functions
  bool create_service(const XML::Element& xml);
  bool create_route(const XML::Element& xml);

public:
  // Factories for use during configuration
  Init::Registry<Service> service_registry;

  //------------------------------------------------------------------------
  // Constructor
  Server() {}

  //------------------------------------------------------------------------
  // Look up a service by id
  Service *lookup_service(const string& id) const;

  //------------------------------------------------------------------------
  // Load modules etc. from XML config
  void configure(const XML::Configuration& config);

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



