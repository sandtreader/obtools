//==========================================================================
// ObTools::XMLMesh: client.cc
//
// Implementation of XMLMesh client library
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xmlmesh-client.h"
#include "ot-log.h"

#include <unistd.h>
#include <sstream>

namespace ObTools { namespace XMLMesh {

//------------------------------------------------------------------------
// Send a message - can block if the queue is full
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
  if (!secondary_q.empty())
  {
    msg = secondary_q.front();
    secondary_q.pop();
    return true;
  }

  // Check if there's anything there
  if (!transport.poll()) return false;

  // Get it
  return wait(msg);
}

//------------------------------------------------------------------------
// Receive a message - blocks waiting for one to arrive
// Returns whether one was read - will only return false if the transport
// was restarted, and messages might have been missed
// Note however subscriptions will be renewed on restart, and you can
// continue to wait for new messages.
bool Client::wait(Message& msg)
{
  // Check if there's anything on the secondary queue first
  if (!secondary_q.empty())
  {
    msg = secondary_q.front();
    secondary_q.pop();
    return true;
  }

  string data;
  if (!transport.wait(data))
  {
    log.summary << "Transport restarted - resubscribing\n";
    resubscribe();
    return false;
  }

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
bool Client::respond(SOAP::Fault::Code code,
		     const string& reason,
		     Message& request)
{
  FaultMessage errm(request.get_id(), code, reason);
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
    log.error << "Sending request failed\n";
    return false;
  }

  bool restarted = false;

  for(;;)
  {
    // Block waiting for a message (note, don't use wait(), otherwise
    // we end up going in circles!)
    string data;
    if (transport.wait(data))
    {
      response = Message(data);

      // Make sure the ref's match
      // Note:  This is the simplest synchronous send/receive;  assumes
      // no interleaving of responses
      if (req.get_id() == response.get_ref())
      {
	// Did we restart earlier - now resubscribe after we've got the
	// result of our original request out of the way
	if (restarted)
	{
	  log.summary << "Resubscribing\n";
	  resubscribe();
	}

	return true;
      }

      // Requeue on the secondary queue so wait() gets it later
      secondary_q.push(response);
    }
    else
    {
      log.summary << "Transport restarted\n";

      // Note resubscribe is required - this will be actioned once we've
      // got our result
      // !Note:  If you don't do this you fall in a heap if the message
      // we're doing is itself a subscription message!
      restarted = true;
    }
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
  FaultMessage errm(response);
  if (!errm)
  {
    log.error << "Weird response received:\n";
    log.error << response << endl;
  }
  else
  {
    log.error << errm << endl;
  }
  return false;
}

//------------------------------------------------------------------------
// Resubscribe for all subjects we should be subscribed to
void Client::resubscribe()
{
  for(list<string>::iterator p = subscribed_subjects.begin();
      p != subscribed_subjects.end();
      ++p)
  {
    SubscriptionMessage msg(SubscriptionMessage::JOIN, *p);
    request(msg);
  }
}

//------------------------------------------------------------------------
// Subscribe for messages of a given subject - expressed as a pattern match
// e.g. client.subscribe("info.*");
// Returns whether successful
bool Client::subscribe(const string& subject)
{
  SubscriptionMessage msg(SubscriptionMessage::JOIN, subject);
  subscribed_subjects.push_back(subject);
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
  subscribed_subjects.remove(subject);
  return request(msg);
}

}} // namespaces




