//==========================================================================
// ObTools::XMLMesh: client.cc
//
// Implementation of XMLMesh client library
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmlmesh-client.h"
#include "ot-log.h"

#include <unistd.h>
#include <netinet/in.h>
#include <sstream>

namespace ObTools { namespace XMLMesh {

//------------------------------------------------------------------------
// Send a message - never blocks, but can fail if the queue is full
// Whether message queued
bool Client::send(Message& msg)
{
  string data = msg.get_text();  // Convert to <message> string
  return transport.send(data);
}

//------------------------------------------------------------------------
// Receive a message - never blocks, returns whether one was received
bool Client::poll(Message& msg)
{
  // Check if there's anything on the secondary queue first
  if (secondary_q.poll())
  {
    msg = secondary_q.wait();
    return true;
  }

  // Check if there's anything there
  if (!transport.poll()) return false;

  // Get it
  return wait(msg);
}

//------------------------------------------------------------------------
// Receive a message - blocks waiting for one to arrive
// Returns whether one was read - will only return false if something fails
bool Client::wait(Message& msg)
{
  // Check if there's anything on the secondary queue first
  if (secondary_q.poll())
  {
    msg = secondary_q.wait();
    return true;
  }

  string data;
  if (!transport.wait(data)) return false;

  msg = Message(data);  // Construct message from XML <message> data
  return true;
}

//------------------------------------------------------------------------
// Return OK to request given
// Returns whether successul
bool Client::respond(Message& request)
{
  OKMessage okm(request.get_id());
  return send(okm);
}

//------------------------------------------------------------------------
// Return an error to request given
// Returns whether successul
bool Client::respond(ErrorMessage::Severity severity,
		     const string& text,
		     Message& request)
{
  ErrorMessage errm(request.get_id(), severity, text);
  return send(errm);
}

//------------------------------------------------------------------------
// Send a message and get a response (blocking)
// Returns whether successful, fills in response if so
bool Client::request(Message& req, Message& response)
{
  // Send message
  if (!send(req))
  {
    Log::Error << "Sending request failed\n";
    return false;
  }

  for(;;)
  {
    // Block waiting for a message (note, don't use wait(), otherwise
    // we end up going in circles!)
    string data;
    if (!transport.wait(data)) 
    {
      Log::Error << "Awaiting response failed\n";
      return false;
    }

    response = Message(data);

    // Make sure the ref's match
    // Note:  This is the simplest synchronous send/receive;  assumes
    // no interleaving of responses
    if (req.get_id() == response.get_ref()) return true;

    // Requeue on the secondary queue so wait() gets it later
    secondary_q.send(response);
  }
}

//------------------------------------------------------------------------
// Send a message and confirm receipt or error
// Returns whether successful.  Handles errors itself
bool Client::request(Message& req)
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
// Subscribe for messages of a given subject - expressed as a pattern match
// e.g. client.subscribe("info.*");
// Returns whether successful
bool Client::subscribe(const string& subject)
{
  SubscriptionMessage msg(SubscriptionMessage::JOIN, subject);
  return request(msg);
}

//------------------------------------------------------------------------
// Unsubscribe for messages of a given subject 
// Subject is a pattern - can use more general pattern to unsubscribe
// more specific ones
// e.g. client.unsubscribe("*");
// Returns whether successful
bool Client::unsubscribe(const string& subject)
{
  SubscriptionMessage msg(SubscriptionMessage::LEAVE, subject);
  return request(msg);
}

}} // namespaces




