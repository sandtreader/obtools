//==========================================================================
// ObTools::XMLMesh:Server: service.cc
//
// Implementation of common parts of services for XMLMesh
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-log.h"
#include "ot-text.h"
#include "server.h"

namespace ObTools { namespace XMLMesh {

//------------------------------------------------------------------------
// Accept a message - implementation of MessageAcceptor
// Performs local processing on messages (optionally in a worker thread)
// and then forwards it to routes (also optionally in worker threads)
void Service::accept(RoutingMessage& msg)
{
  // Decide whether to process it in this thread or a worker
  if (multithreaded)
  {
    ServiceThread *t = threads.remove();
    if (t)
    {
      t->service = this;
      t->msg = &msg;
      t->kick();  // This will handle back in work() in the worker thread
      return;
    }
    else
    {
      // Might get called in another service's thread - use local log 
      Log::Stream error_log(Log::logger, Log::LEVEL_ERROR);
      error_log << "Service " << id << "has no spare threads - serialising\n";
    }
  }

  // If we get to here, there are no (spare) worker threads, so handle it
  // ourselves in this one
  work(msg);
}

//------------------------------------------------------------------------
// Work function - do work on given message within (optional) worker thread
void Service::work(RoutingMessage& msg)
{
  // Call our handler and forward/reverse it if OK
  if (handle(msg))
  {
    if (msg.reversing)
      reverse(msg);
    else
      forward(msg);
  }
}

//------------------------------------------------------------------------
// Forward message to ongoing routes
bool Service::forward(RoutingMessage& msg)
{
  // Get subject
  string subject = msg.message.get_subject();

  // Push our ID onto path
  msg.path.push(id);

  // Loop over all routes to see if they want it
  for(list<MessageRoute>::iterator p=routes.begin();
      p!=routes.end();
      p++)
  {
    MessageRoute& route = *p;

    // Check for subject match - CASE INSENSITIVE
    if (Text::pattern_match(route.subject_pattern, subject, false))
    {
      // Send it to the next service
      route.service.accept(msg);
    }
  }

  // Pop our ID off again
  msg.path.pop();

  return true;
}

//------------------------------------------------------------------------
// Reverse message back to inbound routes
bool Service::reverse(RoutingMessage& msg)
{
  // Pop a service ID off the stack
  string rid = msg.path.pop();

  // Look it up
  Service *rs = server.lookup_service(rid);

  if (rs)
  {
    rs->accept(msg);
    return true;
  }
  else
  {
    Log::Stream error_log(Log::logger, Log::LEVEL_ERROR); 
    error_log << "No such service " << rid << " in reverse routing\n";
    return false;
  }
}

//------------------------------------------------------------------------
// Originate a new message
bool Service::originate(RoutingMessage& msg)
{
  // Forward/Reverse it through routing
  if (msg.reversing)
    return reverse(msg);
  else
    return forward(msg);
}

//------------------------------------------------------------------------
// Return a message as a response to an existing one
// Returns whether successul
bool Service::respond(Message& response, RoutingMessage& request)
{
  // Create a new routing message from us, but with the same path as
  // that received
  ServiceClient client(this, request.client.client);
  RoutingMessage msg(client, response, request.path);
  return originate(msg);
}

//------------------------------------------------------------------------
// Return OK to an existing request
// Returns whether successul
bool Service::respond(RoutingMessage& request)
{
  OKMessage response(request.message.get_id());
  return respond(response, request);
}

//------------------------------------------------------------------------
// Return a fault to an existing request
// Returns whether successul
bool Service::respond(RoutingMessage& request,
		      SOAP::Fault::Code code,
		      const string& reason)
{
  FaultMessage response(request.message.get_id(), code, reason);
  return respond(response, request);
}

//==========================================================================
// ServiceThread worker thread

//------------------------------------------------------------------------
// Run function - pass msg to service 
void ServiceThread::run() 
{ 
  // Take a local copy of the message for this thread to work with
  RoutingMessage localmsg = *msg;  // Copy Constructor!
  service->work(localmsg); 
}

}} // namespaces




