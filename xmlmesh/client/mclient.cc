//==========================================================================
// ObTools::XMLMesh: mclient.cc
//
// Implementation of XMLMesh multi-threaded client
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#include "ot-xmlmesh-client.h"
#include "ot-log.h"
#include "ot-text.h"

#include <unistd.h>
#include <netinet/in.h>
#include <sstream>

namespace ObTools { namespace XMLMesh {

//==========================================================================
// Background traffic handler threads

//--------------------------------------------------------------------------
// Receive handler thread class
// Just repeatedly calls back into receive_messages
class DispatchThread: public MT::Thread
{
  MultiClient& client;
  ClientTransport& transport;

  void run() 
  { 
    for(;;)
    {
      string data;
      if (transport.wait(data))
      {
	Message msg(data);  // Construct message from XML <message> data
	client.dispatch(msg);
      }
      else
      {
	Log::Summary << "Transport restarted - resubscribing\n";
	client.resubscribe();
      }
    }
  }

public:
  DispatchThread(MultiClient &_client, ClientTransport& _transport): 
    client(_client), transport(_transport) { }
};

//------------------------------------------------------------------------
// Dispatch a message (background thread)
void MultiClient::dispatch(Message &msg)
{
  string ref = msg.get_ref();
  MT::RLock l(mutex);

  if (ref.size())
  {
    // Check if it is for one of our requests
    MultiClientRequest *req = requests[ref];
    if (req)
    {
      // Copy in message and release client
      *(req->response) = msg;
      req->done.signal();
    }
    else
    {
      Log::Error << "Response with unknown ref ignored: " << ref << endl;
    }
  }
  else
  {
    string subject = msg.get_subject();
    bool handled = false;

    // Try all the subscribers
    for(list<Subscriber *>::iterator p = subscribers.begin();
	p!=subscribers.end();
	p++)
    {
      Subscriber *sub = *p;
      if (Text::pattern_match(sub->subject, subject))
      {
	sub->handle(msg);
	handled = true;
      }
    }

    // Complain if no-one wanted it
    if (!handled)
      Log::Error << "Unhandled message received with subject " 
		 << subject << endl;
  }
}

//------------------------------------------------------------------------
// Resubscribe for all subjects we should be subscribed to
void MultiClient::resubscribe()
{
  // We take the mutex to lock everything out until this is sorted
  MT::RLock l(mutex);

retry:
  for(list<Subscriber *>::iterator p = subscribers.begin();
      p!=subscribers.end();
      p++)
  {
    Subscriber *sub = *p;
    SubscriptionMessage req(SubscriptionMessage::JOIN, sub->subject);
    string id = req.get_id();

    // Send the message
    send(req);

    // We have to implement our own (sub-)dispatch loop because we are the
    // dispatch thread, and we need a response to the subscribe

    // Loop dispatching message until we get a response to this
    for(;;)
    {
      string data;
      if (transport.wait(data))
      {
	Message msg(data);  // Construct message from XML <message> data

	// Check ref (if any) to see if it's ours
	string ref = msg.get_ref();
	if (ref == id)
	{
	  // Check it for OK
	  if (msg.get_subject() == "xmlmesh.ok") break;

	  Log::Error << "Error response to resubscribe:\n" << data << endl;
	}
	else
	{
	  // Dispatch to others and continue
	  dispatch(msg);
	}
      }
      else
      {
	Log::Summary << "Transport failed during resubscribe - retrying\n";
	goto retry;  // Try it all again
      }
    }
  }
}

//==========================================================================
// Foreground stuff

//------------------------------------------------------------------------
// Constructor - attach transport
MultiClient::MultiClient(ClientTransport& _transport): transport(_transport)
{
  dispatch_thread = new DispatchThread(*this, transport);
}

//------------------------------------------------------------------------
// Start - allows transport-specific child class to ensure transport
// is initialised before doing anything with it
void MultiClient::start()
{
  dispatch_thread->start();
}

//------------------------------------------------------------------------
// Send a message - never blocks, but can fail if the queue is full
// Whether message queued
bool MultiClient::send(Message& msg)
{
  string data = msg.get_text();  // Convert to <message> string
  return transport.send(data);
}

//------------------------------------------------------------------------
// Return OK to request given
// Returns whether successul
bool MultiClient::respond(Message& request)
{
  OKMessage okm(request.get_id());
  return send(okm);
}

//------------------------------------------------------------------------
// Return an error to request given
// Returns whether successul
bool MultiClient::respond(ErrorMessage::Severity severity,
			  const string& text,
			  Message& request)
{
  ErrorMessage errm(request.get_id(), severity, text);
  return send(errm);
}

//------------------------------------------------------------------------
// Send a message and get a response (blocking)
// Returns whether successful, fills in response if so
bool MultiClient::request(Message& req, Message& response)
{
  // Create a client request
  MultiClientRequest mcr(&response);

  // Register it
  string id = req.get_id();
  {
    MT::RLock l(mutex);
    requests[id] = &mcr;
  }

  // Send message
  if (!send(req))
  {
    Log::Error << "Sending request failed\n";
    MT::RLock l(mutex);
    requests.erase(id);
    return false;
  }

  // Block on response flag
  mcr.done.wait();

  // Remove it from map 
  {
    MT::RLock l(mutex);
    requests.erase(id);
  }

  return true;
}

//------------------------------------------------------------------------
// Send a message and confirm receipt or error
// Returns whether successful.  Handles errors itself
bool MultiClient::request(Message& req)
{
  Message response;
  if (!request(req, response)) return false;

  // Check type of message
  string subject = response.get_subject();
  if (subject == "xmlmesh.ok") return true;

  // Handle error
  ErrorMessage errm(response);
  if (!errm)
  {
    Log::Error << "Weird response received:\n";
    Log::Error << response << endl;
  }
  else
  {
    Log::Error << "Request error:\n";
    Log::Error << errm << endl;
  }
  return false;
}

//------------------------------------------------------------------------
// Register a subscriber functor
void MultiClient::register_subscriber(Subscriber *sub)
{
  {
    MT::RLock l(mutex);
    subscribers.push_back(sub);
  }

  // Subscribe for that subject
  SubscriptionMessage msg(SubscriptionMessage::JOIN, sub->subject);
  if (!request(msg))
    Log::Error << "Unable to subscribe for " << sub->subject << endl;
}

//------------------------------------------------------------------------
// Deregister a subscriber functor
void MultiClient::deregister_subscriber(Subscriber *sub)
{
  // Unsubscribe for that subject
  SubscriptionMessage msg(SubscriptionMessage::LEAVE, sub->subject);
  if (!request(msg))
    Log::Error << "Unable to unsubscribe for " << sub->subject << endl;

  {
    MT::RLock l(mutex);
    subscribers.remove(sub);
  }
}

//------------------------------------------------------------------------
// Destructor
MultiClient::~MultiClient()
{
  delete dispatch_thread;
}

//==========================================================================
// Subscriber stuff

//------------------------------------------------------------------------
// Constructor - register into client
Subscriber::Subscriber(MultiClient& _client, const string& _subject):
  client(_client), subject(_subject)
{
  client.register_subscriber(this);
}

//------------------------------------------------------------------------
// Destructor - deregister from client
Subscriber::~Subscriber()
{
  client.deregister_subscriber(this);
}

}} // namespaces




