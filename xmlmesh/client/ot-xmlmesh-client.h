//==========================================================================
// ObTools::XMLMesh:Client: ot-xmlmesh-client.h
//
// Public definitions for XMLMesh client library
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_XMLMESH_CLIENT_H
#define __OBTOOLS_XMLMESH_CLIENT_H

#include <string>
#include <queue>
#include "ot-net.h"
#if !defined(_SINGLE)
#include "ot-mt.h"
#endif
#include "ot-xmlmesh.h"

namespace ObTools { namespace XMLMesh {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Client Transport (abstract interface)
// Low-level transport of raw data
class ClientTransport
{
public:
  // Send a message - returns whether successful
  virtual bool send(const string& data) = 0;

  // Check whether a message is available before blocking in wait()
  virtual bool poll() = 0;

  // Wait for a message - blocking.
  // Returns false if the transport was restarted and subscriptions
  // (and messages) may have been lost
  virtual bool wait(string& data) = 0;
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

  void resubscribe();

public:
  //------------------------------------------------------------------------
  // Constructor - attach transport
  Client(ClientTransport& _transport): transport(_transport) {}

  //------------------------------------------------------------------------
  // Send a message - never blocks, but can fail if the queue is full
  // Whether message queued
  bool send(Message& msg);

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
  bool respond(Message& request);

  //------------------------------------------------------------------------
  // Return an error to request given
  // Returns whether successul
  bool respond(ErrorMessage::Severity severity,
	       const string& text,
	       Message& request);

  //------------------------------------------------------------------------
  // Send a message and get a response (blocking)
  // Returns whether successful, fills in response if so
  bool request(Message& req, Message& response);

  //------------------------------------------------------------------------
  // Send a message and confirm receipt or error
  // Returns whether successful.  Handles errors itself
  bool request(Message& req);

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

#if !defined(_SINGLE)
class MultiClient;  // forward
//==========================================================================
// Subscriber objects - abstract functors to handle callback from MultiClient
// for subscribed messages (see below), also with the subscribe/unsubscribe
// functionality of Subscription above
// See mclient.cc
class Subscriber
{
protected:
  MultiClient& client;

public:
  string subject;

  //------------------------------------------------------------------------
  // Constructor/destructor register/unregister into multiclient
  Subscriber(MultiClient& _client, const string& _subject);
  virtual ~Subscriber();

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
  MT::RMutex mutex;                            // Global state lock

public:
  //------------------------------------------------------------------------
  // Constructor - attach transport
  MultiClient(ClientTransport& _transport);

  //------------------------------------------------------------------------
  // Start - allows transport-specific child class to ensure transport
  // is initialised before doing anything with it
  void start();

  //------------------------------------------------------------------------
  // Send a message - never blocks, but can fail if the queue is full
  // Whether message queued
  bool send(Message& msg);

  //------------------------------------------------------------------------
  // Return OK to request given
  // Returns whether successul
  bool respond(Message& request);

  //------------------------------------------------------------------------
  // Return an error to request given
  // Returns whether successul
  bool respond(ErrorMessage::Severity severity,
	       const string& text,
	       Message& request);

  //------------------------------------------------------------------------
  // Send a message and get a response (blocking)
  // Returns whether successful, fills in response if so
  bool request(Message& req, Message& response);

  //------------------------------------------------------------------------
  // Send a message and confirm receipt or error
  // Returns whether successful.  Handles errors itself
  bool request(Message& req);

  //------------------------------------------------------------------------
  // Register a subscriber functor
  void register_subscriber(Subscriber *sub);

  //------------------------------------------------------------------------
  // Deregister a subscriber functor
  void deregister_subscriber(Subscriber *sub);

  //------------------------------------------------------------------------
  // Dispatch a message (background thread)
  void dispatch(Message &msg);

  //------------------------------------------------------------------------
  // Resubscribe for all subjects we should be subscribed to (background)
  void resubscribe();

  //------------------------------------------------------------------------
  // Destructor
  ~MultiClient();
};

#endif // !_SINGLE

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_XMLMESH_CLIENT_H



