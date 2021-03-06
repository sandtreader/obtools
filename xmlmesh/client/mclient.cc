//==========================================================================
// ObTools::XMLMesh: mclient.cc
//
// Implementation of XMLMesh multi-threaded client
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xmlmesh-client.h"
#include "ot-log.h"
#include "ot-text.h"
#include "ot-time.h"

#include <unistd.h>
#include <sstream>

// Define for message handler timing - note that it actually outputs at
// 'detail' level, so you can get timing from a debug version without
// all the rest of the debugging
#if defined(DEBUG)
#define TIME_MESSAGES 1
#endif

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
  Log::Streams log;  // Thread-local log streams

  void run()
  {
    for(;;)
    {
      string data;
      if (transport.wait(data))
      {
        Message msg(data);  // Construct message from XML <message> data
        client.handle(msg, log);
      }
      else
      {
        // Stop on shutdown
        if (!client.is_alive()) break;

        log.summary << "Transport restarted - resubscribing\n";
        client.resubscribe(log);
      }
    }
  }

public:
  DispatchThread(MultiClient &_client, ClientTransport& _transport):
    client(_client), transport(_transport) { }
};

//--------------------------------------------------------------------------
// Handle a message (background thread)
void MultiClient::handle(Message &msg, Log::Streams& log)
{
  string ref = msg.get_ref();

  if (ref.size())
  {
    MT::RLock l(mutex);

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
      log.error << "Response with unknown ref ignored: " << ref << endl;
    }
  }
  else
  {
    // Pass it to a worker thread - this allows the subscriber to
    // initiate an outgoing request and get the result - otherwise
    // we end up in deadlock
    MultiClientWorker *t = workers.remove();

    if (t)
    {
      t->client = this;
      t->msg = new Message(msg);  // Copy for safety
      t->kick();  // This will handle back in run() in the worker thread
    }
    else
    {
      log.error << "Mesh client has no spare workers - '"
                << msg.get_subject() << "' message queued\n";

      // Queue a copy for an existing worker to catch
      pending_queue.send(new Message(msg));
    }
  }
}

//--------------------------------------------------------------------------
// Dispatch a message (worker thread)
void MultiClient::dispatch(Message *msg)
{
  // May loop to handle pending message
  for(;;)
  {
    string subject = msg->get_subject();
    list<Subscriber *> handlers;

    {
      // Lock while reading subscriber list
      MT::RLock l(mutex);

      // Try all the subscribers
      for(list<Subscriber *>::iterator p = subscribers.begin();
          p!=subscribers.end();
          )
      {
        list<Subscriber *>::iterator q = p++;  // Protect from deletion
        Subscriber *sub = *q;

        // This is where we kill dead ones
        if (sub->dead && !sub->active)
        {
          delete sub;
          subscribers.erase(q);
        }
        else if (Text::pattern_match(sub->subject, subject))
        {
          sub->active++;  // Protected by global lock
          handlers.push_back(sub);
        }
      }
    }

    // Complain if no-one wanted it
    if (handlers.empty())
    {
      Log::Streams log;
      log.error << "Unhandled message received with subject "
                << subject << endl;
    }

    // Now have own safe list of subscribers with active set - no-one can
    // interfere with this, even if they try unsubscribing now
    for(list<Subscriber *>::iterator p = handlers.begin();
        p!=handlers.end();
        p++)
    {
      Subscriber *sub = *p;
#if defined(TIME_MESSAGES)
      Time::Stamp start = Time::Stamp::now();
#endif

      sub->handle(*msg);

#if defined(TIME_MESSAGES)
      Time::Duration taken = Time::Stamp::now() - start;
      Log::Streams log;
      log.detail << "Incoming message '" << subject << "' handled in "
                 << taken.seconds() << "s\n";
#endif
    }

    // Deactivate subscribers again inside global lock
    {
      MT::RLock l(mutex);

      for(list<Subscriber *>::iterator p = handlers.begin();
          p!=handlers.end();
          p++)
      {
        Subscriber *sub = *p;
        sub->active--;
      }
    }

    delete msg;

    // Check for pending messages and handle them immediately
    if (pending_queue.poll())
      msg = pending_queue.wait();
    else
      break;
  }
}

//--------------------------------------------------------------------------
// Resubscribe for all subjects we should be subscribed to
void MultiClient::resubscribe(Log::Streams& log)
{
  // We take the mutex to lock everything out until this is sorted
  MT::RLock l(mutex);

retry:
  for(list<Subscriber *>::iterator p = subscribers.begin();
      p!=subscribers.end();
      p++)
  {
    Subscriber *sub = *p;
    if (sub->dead) continue;

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

          log.error << "Error response to resubscribe:\n" << data << endl;
        }
        else
        {
          // Dispatch to others and continue
          handle(msg, log);
        }
      }
      else
      {
        log.summary << "Transport failed during resubscribe - retrying\n";
        goto retry;  // Try it all again
      }
    }
  }
}

//==========================================================================
// MultiClientWorker worker thread

//--------------------------------------------------------------------------
// Run function - pass msg to subscriber
void MultiClientWorker::run()
{
  client->dispatch(msg);
}

//==========================================================================
// Foreground stuff

//--------------------------------------------------------------------------
// Constructor - attach transport
MultiClient::MultiClient(ClientTransport& _transport,
                         int _min_spare_workers, int _max_workers):
  transport(_transport), workers(_min_spare_workers, _max_workers),
  alive(true), shutting_down(false)
{
  dispatch_thread = new DispatchThread(*this, transport);
}

//--------------------------------------------------------------------------
// Start - allows transport-specific child class to ensure transport
// is initialised before doing anything with it
void MultiClient::start()
{
  dispatch_thread->start();
}

//--------------------------------------------------------------------------
// Send a message - never blocks, but can fail if the queue is full
// Whether message queued
bool MultiClient::send(const Message& msg)
{
  string data = msg.get_text();  // Convert to <message> string
  return transport.send(data);
}

//--------------------------------------------------------------------------
// Return OK to request given
// Returns whether successul
bool MultiClient::respond(const Message& request)
{
  OKMessage okm(request.get_id());
  return send(okm);
}

//--------------------------------------------------------------------------
// Return an error to request given
// Returns whether successul
bool MultiClient::respond(SOAP::Fault::Code code,
                          const string& reason,
                          const Message& request)
{
  FaultMessage errm(request.get_id(), code, reason);
  return send(errm);
}

//--------------------------------------------------------------------------
// Send a message and get a response (blocking)
// Returns whether successful, fills in response if so
bool MultiClient::request(const Message& req, Message& response)
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
    Log::Error error_log;
    error_log << "Sending request failed\n";
    MT::RLock l(mutex);
    requests.erase(id);
    return false;
  }

#if defined(TIME_MESSAGES)
  Time::Stamp start = Time::Stamp::now();
#endif

  // Block on response flag
  mcr.done.wait();

#if defined(TIME_MESSAGES)
  Time::Duration taken = Time::Stamp::now() - start;
  Log::Streams log;
  log.detail << "Outgoing message '" << req.get_subject()
             << "' responded to in "
             << taken.seconds() << "s\n";
#endif

  // Remove it from map
  {
    MT::RLock l(mutex);
    requests.erase(id);
  }

  return true;
}

//--------------------------------------------------------------------------
// Send a message and confirm receipt or error
// Returns whether successful.  Handles errors itself
bool MultiClient::request(const Message& req)
{
  Message response;
  if (!request(req, response)) return false;

  // Check type of message
  string subject = response.get_subject();
  if (subject == "xmlmesh.ok") return true;

  // Handle error
  FaultMessage errm(response);
  Log::Error error_log;
  if (!errm)
  {
    error_log << "Weird response received:\n";
    error_log << response << endl;
  }
  else
  {
    error_log << errm << endl;
  }
  return false;
}

//--------------------------------------------------------------------------
// Register a subscriber functor
void MultiClient::register_subscriber(Subscriber *sub)
{
  {
    MT::RLock l(mutex);
    subscribers.push_back(sub);
  }

  // If the transport isn't connected we just give up because when it
  // comes back we will resubscribe, and blocking here would lock up
  // the application
  if (!transport.is_connected())
  {
    Log::Error error_log;
    error_log << "XMLMesh not connected - delaying subscription for "
              << sub->subject << endl;
    return;
  }

  // Subscribe for that subject
  SubscriptionMessage msg(SubscriptionMessage::JOIN, sub->subject);
  if (!request(msg))
  {
    Log::Error error_log;
    error_log << "Unable to subscribe for " << sub->subject << endl;
  }
}

//--------------------------------------------------------------------------
// Deregister a subscriber functor
void MultiClient::deregister_subscriber(Subscriber *sub)
{
  // Unsubscribe for that subject
  SubscriptionMessage msg(SubscriptionMessage::LEAVE, sub->subject);

  // If shutting down, don't wait for a response - otherwise we can get
  // blocked waiting for OTMP socket to recover, when we don't really care
  if (shutting_down?!send(msg):!request(msg))
  {
    Log::Error error_log;
    error_log << "Unable to unsubscribe for " << sub->subject << endl;
  }

  sub->dead = true;  // Trigger deletion in dispatch() when safe
}

//--------------------------------------------------------------------------
// Prepare for shutdown
void MultiClient::prepare_shutdown()
{
  shutting_down = true;
}

//--------------------------------------------------------------------------
// Clean shutdown
void MultiClient::shutdown()
{
  if (alive)
  {
    alive = false;
    transport.shutdown();
    if (dispatch_thread) delete dispatch_thread;
    dispatch_thread = 0;

    // Delete dead subscribers
    // (undead ones, leave and complain when the user kills them)
    for(auto p: subscribers)
    {
      if (p->dead)
        delete p;
      else
        cerr << "XMLMesh subscriber not disconnected\n";
    }
  }
}

//--------------------------------------------------------------------------
// Destructor
MultiClient::~MultiClient()
{
  shutdown();
}

//==========================================================================
// Subscriber stuff

//--------------------------------------------------------------------------
// Constructor - register into client
Subscriber::Subscriber(MultiClient& _client, const string& _subject):
  client(_client), subject(_subject), active(0), dead(false)
{
  client.register_subscriber(this);
}

//--------------------------------------------------------------------------
// Manual unsubscription.  Always use this to unsubscribe and delete a
// dynamic subscription in preference to the destructor.  If you call this,
// don't delete the Subscriber yourself - it will be done when it is safe
// to do so.
void Subscriber::disconnect()
{
  client.deregister_subscriber(this);
}

//--------------------------------------------------------------------------
// Destructor - deregister from client
Subscriber::~Subscriber()
{
  if (!dead)
  {
    // Enforce disconnect()
    cerr << "XMLMesh Subscriber deleted through destructor, not disconnect\n";
    abort();
  }
}

}} // namespaces




