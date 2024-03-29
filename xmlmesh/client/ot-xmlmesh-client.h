//==========================================================================
// ObTools::XMLMesh:Client: ot-xmlmesh-client.h
//
// Public definitions for XMLMesh client library
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_XMLMESH_CLIENT_H
#define __OBTOOLS_XMLMESH_CLIENT_H

#include <string>
#include <queue>
#include "ot-net.h"
#include "ot-log.h"
#include "ot-mt.h"
#include "ot-msg.h"
#include "ot-xmlmesh.h"

namespace ObTools { namespace XMLMesh {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Client Transport (abstract interface)
// Low-level transport of raw data
class ClientTransport
{
public:
  //------------------------------------------------------------------------
  // Check if the transport is connected
  virtual bool is_connected() = 0;

  //------------------------------------------------------------------------
  // Send a message - returns whether successful
  virtual bool send(const string& data) = 0;

  //------------------------------------------------------------------------
  // Check whether a message is available before blocking in wait()
  virtual bool poll() = 0;

  //------------------------------------------------------------------------
  // Wait for a message - blocking.
  // Returns false if the transport was restarted and subscriptions
  // (and messages) may have been lost
  virtual bool wait(string& data) = 0;

  //------------------------------------------------------------------------
  // Clean shutdown (optional)
  virtual void shutdown() {};

  //------------------------------------------------------------------------
  // Virtual destructor - does nothing here
  virtual ~ClientTransport() {}
};

//==========================================================================
// Simple 'single-user' XMLMesh client using any transport
class Client
{
private:
  ClientTransport& transport;
  queue<Message> secondary_q;  // Queue for unwanted messages received
                               // in request()
  list<string> subscribed_subjects;
  Log::Streams log;            // Private (threadsafe) log streams
  void resubscribe();

public:
  //------------------------------------------------------------------------
  // Constructor - attach transport
  Client(ClientTransport& _transport): transport(_transport) {}

  //------------------------------------------------------------------------
  // Send a message - can block if queue full
  // Whether message queued
  bool send(const Message& msg);

  //------------------------------------------------------------------------
  // Receive a message - never blocks, returns whether one was received
  bool poll(Message& msg);

  //------------------------------------------------------------------------
  // Receive a message - blocks waiting for one to arrive
  // Returns whether one was read - will only return false if the transport
  // was restarted, and messages might have been missed
  // Note however subscriptions will be renewed on restart, and you can
  // continue to wait for new messages.
  bool wait(Message& msg);

  //------------------------------------------------------------------------
  // Return OK to request given
  // Returns whether successul
  bool respond(const Message& request);

  //------------------------------------------------------------------------
  // Return an error to request given
  // Returns whether successul
  bool respond(SOAP::Fault::Code code,
               const string& reason,
               const Message& request);

  //------------------------------------------------------------------------
  // Send a message and get a response (blocking)
  // Returns whether successful, fills in response if so
  bool request(const Message& req, Message& response);

  //------------------------------------------------------------------------
  // Send a message and confirm receipt or error
  // Returns whether successful.  Handles errors itself
  bool request(const Message& req);

  //------------------------------------------------------------------------
  // Subscribe for messages of a given subject - expressed as a pattern match
  // e.g. client.subscribe("info.*");
  // Returns whether successful
  bool subscribe(const string& subject);

  //------------------------------------------------------------------------
  // Unsubscribe for messages of a given subject
  // Subject is a pattern - can use more general pattern to unsubscribe
  // more specific ones
  // e.g. client.unsubscribe("*");
  // Returns whether successful
  bool unsubscribe(const string& subject);

  //------------------------------------------------------------------------
  // Shut down cleanly
  void shutdown() { transport.shutdown(); }
};

//==========================================================================
// Subscription class - subscribes for given subject on given single-user
// client
class Subscription
{
private:
  Client& client;
  string subject;

public:
  //------------------------------------------------------------------------
  // Constructor/destructor simply subscribe/unsubscribe
  Subscription(Client& _client, const string& _subject):
    client(_client), subject(_subject)
  { client.subscribe(subject); }

  ~Subscription() { client.unsubscribe(subject); }
};

class MultiClient;  // forward
//==========================================================================
// Subscriber objects - abstract functors to handle callback from MultiClient
// for subscribed messages (see below), also with the subscribe/unsubscribe
// functionality of Subscription above
// See mclient.cc

// NOTE!  You must not delete this while handling a message through it
// - evidently, in terms of C++ object safety, but also there is a
// mutex set while handling a message to prevent deletion by another
// thread.  Also, if there is any chance that a message could arrive, or
// already be active, through another thread while deleting it (which
// is usually the case with dynamic subscriptions) call disconnect()
// instead

// ** THIS MEANS SUBSCRIBER OBJECTS SHOULD NEVER BE INCLUDED OR AUTO  **
// ** only new()'ed, and disconnected()'ed rather than delete()'ed    **

// [Technical rationale:  This object is both the subscription and the
//  message handler.  The destructor can only unsubscribe once the
//  Subscriber superclass destructor is called, by which time the object has
//  'become' just a Subscriber, with no implementation of handle(), so you
//  will get 'pure virtual method called' aborts() if a message is already
//  in progress.  In any case, your child class state is no longer valid at
//  that point.  disconnect() safely deregisters and requests a delayed
//  death if anything (including the caller) is active inside the handler
class Subscriber
{
protected:
  MultiClient& client;

public:
  string subject;
  int active;        // Number of handlers using this
  bool dead;         // Death requested

  //------------------------------------------------------------------------
  // Constructor: register into multiclient
  Subscriber(MultiClient& _client, const string& _subject);
  virtual ~Subscriber();

  //------------------------------------------------------------------------
  // Manual unsubscription.  Always use this to unsubscribe and delete a
  // dynamic subscription in preference to the destructor.  If you call this,
  // don't delete the Subscriber yourself - it will be done when it is safe
  // to do so.
  void disconnect();

  //------------------------------------------------------------------------
  // Message handler function - implemented by child classes
  virtual void handle(Message& msg) = 0;
};

//==========================================================================
// Request-response request record
struct MultiClientRequest
{
  Message *response;    // Where to put response message
  MT::Condition done;   // Whether done

  MultiClientRequest(Message *_response): response(_response) {}
};

//==========================================================================
// Worker thread for multi-threaded subscribers
class MultiClientWorker: public MT::PoolThread
{
public:
  MultiClient *client;
  Message *msg;

  MultiClientWorker(ObTools::MT::PoolReplacer<MultiClientWorker>& _rep):
    PoolThread(_rep) {}

  //------------------------------------------------------------------------
  // Run function - pass msg to subscriber
  void run();
};

//==========================================================================
// Complex 'multi-user' XMLMesh client using any transport
// Operates with background worker threads
// Request-response is handled with the same interface as Client, but
// multiple may be outstanding in different threads at once
// Subscribed messages are delivered into Subscriber objects (q.v.)
class MultiClient
{
private:
  ClientTransport& transport;
  list<Subscriber *> subscribers;              // Active subscribers
  map<string, MultiClientRequest *> requests;  // Active requests, by id
  MT::Thread *dispatch_thread;                 // Message dispatch thread
  MT::ThreadPool<MultiClientWorker> workers;   // Worker thread pool
  MT::Queue<Message *> pending_queue;          // Queued messages
  MT::RMutex mutex;                            // Global state lock
  bool alive;
  bool shutting_down;

public:
  static const int DEFAULT_MIN_SPARE_WORKERS = 1;
  static const int DEFAULT_MAX_WORKERS = 10;

  //------------------------------------------------------------------------
  // Constructor - attach transport
  MultiClient(ClientTransport& _transport,
              int _min_spare_workers=DEFAULT_MIN_SPARE_WORKERS,
              int _max_workers=DEFAULT_MAX_WORKERS);

  //------------------------------------------------------------------------
  // Check if not shutting down
  bool is_alive() { return alive; }

  //------------------------------------------------------------------------
  // Start - allows transport-specific child class to ensure transport
  // is initialised before doing anything with it
  void start();

  //------------------------------------------------------------------------
  // Send a message - can block if queue is full
  // Whether message queued
  bool send(const Message& msg);

  //------------------------------------------------------------------------
  // Return OK to request given
  // Returns whether successul
  bool respond(const Message& request);

  //------------------------------------------------------------------------
  // Return an error to request given
  // Returns whether successul
  bool respond(SOAP::Fault::Code code,
               const string& reason,
               const Message& request);

  //------------------------------------------------------------------------
  // Send a message and get a response (blocking)
  // Returns whether successful, fills in response if so
  bool request(const Message& req, Message& response);

  //------------------------------------------------------------------------
  // Send a message and confirm receipt or error
  // Returns whether successful.  Handles errors itself
  bool request(const Message& req);

  //------------------------------------------------------------------------
  // Register a subscriber functor
  void register_subscriber(Subscriber *sub);

  //------------------------------------------------------------------------
  // Deregister a subscriber functor
  void deregister_subscriber(Subscriber *sub);

  //------------------------------------------------------------------------
  // Handle a message (background thread)
  void handle(Message &msg, Log::Streams& log);

  //------------------------------------------------------------------------
  // Dispatch a message (worker thread)
  void dispatch(Message *msg);

  //------------------------------------------------------------------------
  // Resubscribe for all subjects we should be subscribed to (background)
  void resubscribe(Log::Streams& log);

  //------------------------------------------------------------------------
  // Prepare for shutdown
  void prepare_shutdown();

  //------------------------------------------------------------------------
  // Clean shutdown
  void shutdown();

  //------------------------------------------------------------------------
  // Destructor
  virtual ~MultiClient();
};

//==========================================================================
// Mesh Transport subscriber for use with message Transport below
template<class CONTEXT>
class MessageTransportSubscriber: public Subscriber
{
  // Context to work in
  CONTEXT& context;

  // Message handler to send message to
  ObTools::Message::Handler<CONTEXT>& message_handler;

 public:

  //------------------------------------------------------------------------
  // Constructor - takes URL pattern
  MessageTransportSubscriber(CONTEXT& _context,
                             const string& _subject,
                             MultiClient& client,
                             ObTools::Message::Handler<CONTEXT>& _handler):
    Subscriber(client, _subject+".request*"), context(_context),
    message_handler(_handler)
  {}

  //------------------------------------------------------------------------
  // Handle a message
  void handle(Message& msg)
  {
    const XML::Element& request = msg.get_body();

    // Check body document name, if specified
    if (!message_handler.document_name.empty()
     && request.name != message_handler.ns_prefix + ":"
                      + message_handler.document_name+"-request")
    {
      client.respond(SOAP::Fault::CODE_SENDER, "Bad document name", msg);
      return;
    }

    // Prepare response body, even if not used
    XML::Element response(message_handler.ns_prefix+":"
                          +message_handler.document_name+"-response",
                          "xmlns:" + message_handler.ns_prefix,
                          message_handler.ns_url);

    // Create fake SSL client details
    Net::EndPoint ep;
    SSL::ClientDetails ssl(ep, "#xmlmesh");  // Fake

    // Get handler to deal with message
    try
    {
      message_handler.handle_message(context, request, ssl, response);

      if (message_handler.complex_result)
      {
        // Change .request to .response
        // Assuming it's the last occurance of .request, but perhaps not the
        // end of the subject
        auto resp_subject = msg.get_subject();
        const auto pos = resp_subject.rfind(".request");
        if (pos != string::npos)
          resp_subject.replace(pos, 8, ".response");

        // Send correlated response
        XMLMesh::Message resp(resp_subject, response, false, msg.get_id());
        client.send(resp);
      }
      else
      {
        // Simple OK
        client.respond(msg);
      }
    }
    catch (const ObTools::Message::Exception& me)
    {
      Log::Error log;
      log << "XMLMesh request failed: " << me.what() << endl;
      client.respond(SOAP::Fault::CODE_SENDER, me.what(), msg);
    }
  }
};

//==========================================================================
// Mesh Transport class for use with Message::Broker
template<class CONTEXT>
class MessageTransport: public ObTools::Message::Transport<CONTEXT>
{
  CONTEXT& context;
  MultiClient& client;
  list<XMLMesh::Subscriber *> subscribers;

 public:
  //------------------------------------------------------------------------
  MessageTransport(CONTEXT& _context, MultiClient& _client):
    ObTools::Message::Transport<CONTEXT>("xmlmesh"), context(_context),
    client(_client)
  {}

  //------------------------------------------------------------------------
  // Register a handler with the given config element
  virtual void register_handler(ObTools::Message::Handler<CONTEXT>& handler,
                                const XML::Element& config)
  {
    // Get subject
    string subject = config["subject"];

    // Create URL handler
    MessageTransportSubscriber<CONTEXT> *sub =
      new MessageTransportSubscriber<CONTEXT>(context, subject,
                                              client, handler);
    subscribers.push_back(sub);
  }

  //------------------------------------------------------------------------
  // Destructor
  ~MessageTransport()
  {
    // Tell client to prepare to shut down
    client.prepare_shutdown();

    // Destroy subscribers
    // Note MultiClient shutdown() deletes them
    for(list<XMLMesh::Subscriber *>::iterator p = subscribers.begin();
        p != subscribers.end(); ++p)
      (*p)->disconnect();
  }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLMESH_CLIENT_H



