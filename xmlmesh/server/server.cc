//==========================================================================
// ObTools::XMLMesh:Server server.cc
//
// Implementation of XMLMesh server object
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "server.h"
#include "ot-log.h"
#include <time.h>

#define CORRELATOR_TICK_DELAY 30

namespace ObTools { namespace XMLMesh { 

//------------------------------------------------------------------------
// Constructor
Server::Server(): correlator(*this)
{

}

//------------------------------------------------------------------------
// Register a transport type
void Server::register_transport(const string& name, 
				TransportFactory *factory)
{
  transport_factories[name] = factory;
}

//------------------------------------------------------------------------
// Register a service type
void Server::register_service(const string& name, ServiceFactory *factory)
{
  service_factories[name] = factory;
}

//------------------------------------------------------------------------
// Load modules etc. from XML config
void Server::configure(XML::Configuration& config)
{
  XML::Element& root = config.get_root();

  // Read all transports
  XML::Element& transports = root.get_child("transports");
  
  // Read all sub-elements
  OBTOOLS_XML_FOREACH_CHILD(te, transports)
    if (!create_transport(te))
      Log::Error << "Failed to create transport from XML:\n" << te;
  OBTOOLS_XML_ENDFOR

  // Read all services
  XML::Element& services = root.get_child("services");
  
  // Read all sub-elements
  OBTOOLS_XML_FOREACH_CHILD(se, services)
    if (!create_service(se))
      Log::Error << "Failed to create service from XML:\n" << se;
  OBTOOLS_XML_ENDFOR
}

//------------------------------------------------------------------------
// Create a new transport from the given XML
// Returns whether successful
bool Server::create_transport(XML::Element& xml)
{
  // Find tag name in registry
  map<string, TransportFactory *>::iterator p;
  p = transport_factories.find(xml.name);
  if (p==transport_factories.end()) return false;

  // Call factory to create transport
  TransportFactory *factory = p->second;
  Transport *t = factory->create(xml);
  
  // Connect to our incoming queue and store it
  t->attach_incoming(incoming_q);
  transports.push_back(t);

  // Map its id, only if it has one
  string id = xml.get_attr("id");
  if (id.size()) transport_ids[id] = t;

  return true;
}

//------------------------------------------------------------------------
// Create a new service from the given XML
// Returns whether successful
bool Server::create_service(XML::Element& xml)
{
  // Find tag name in registry
  map<string, ServiceFactory *>::iterator p;
  p = service_factories.find(xml.name);
  if (p==service_factories.end()) return false;

  // Call factory to create service
  ServiceFactory *factory = p->second;
  Service *s = factory->create(*this, xml);
  
  // Store it
  services.push_back(s);

  return true;
}

//--------------------------------------------------------------------------
// Server run method
// Pull messages off receive queue and distributes them
void Server::run() 
{ 
  time_t last_tick = time(0);

  for(;;)
  {
    // Block for a message
    IncomingMessage msg = incoming_q.wait();

    if (msg.flags & IncomingMessage::STARTED)
    {
      // Log it
      Log::Summary << "Client " << msg.transport->name
		   << ":" << msg.client << " starting\n"; 

      // Signal start of client 
      signal_services(msg.transport, msg.client, Service::CLIENT_STARTED);
    }
    else if (msg.flags & IncomingMessage::FINISHED)
    {
      // Log it
      Log::Summary << "Client " << msg.transport->name
		   << ":" << msg.client << " finished\n"; 

      // Signal end of client 
      signal_services(msg.transport, msg.client, Service::CLIENT_FINISHED);
    }
    else
    {
      // Log it
      Log::Detail << "Received message from " << msg.transport->name
		  << ":" << msg.client << ", subject " 
		  << msg.message.get_subject() << ":\n";
      Log::Detail << msg.message.to_text() << endl;

      // Handle request-response pairs
      string ref = msg.message.get_ref();
      if (ref.size())
      {
	Log::Detail << "Response to message ID " << ref << endl;
	correlator.handle_response(msg);
      }
      else if (msg.message.get_rsvp())
      {
	Log::Detail << "Response requested, ID is " 
		    << msg.message.get_id() << endl;
      }

      // Deal with it
      distributor.distribute(msg);

      // If response was required and no-one did, respond with our own error
      if (msg.message.get_rsvp() && !(msg.flags & IncomingMessage::RESPONDED))
	respond(ErrorMessage::ERROR, "No response available", msg);
    }

    // Tick correlator timeout if we haven't done it for a while
    time_t now = time(0);
    if (now-last_tick >= CORRELATOR_TICK_DELAY)
    {
      correlator.tick();
      last_tick = now;
    }
  }
}

//------------------------------------------------------------------------
// Look up a transport by name 
Transport *Server::lookup_transport(const string& name)
{
  map<string, Transport *>::iterator p = transport_ids.find(name);
  if (p!=transport_ids.end())
    return p->second;
  else
    return 0;
}

//------------------------------------------------------------------------
// Signal all services that a client has started or finished
void Server::signal_services(Transport *transport, Net::EndPoint& client,
			     Service::Signal signal)
{
  for(list<Service *>::iterator p=services.begin();
      p!=services.end();
      p++)
    (*p)->signal_client(transport, client, signal);
}

//------------------------------------------------------------------------
// Attach a new message handler on the given subject pattern
// Handlers will be deleted on destruction
void Server::attach_handler(const string& subject, MessageHandler& h)
{
  distributor.attach_handler(subject, h);
}

//------------------------------------------------------------------------
// Send a new message to the client on the transport given
// Returns whether successful
bool Server::send(Message &msg, Transport *transport, Net::EndPoint& client)
{
  transport->send(client, msg.get_text());
}

//------------------------------------------------------------------------
// Return a message as a response to the request given
// Returns whether successul
bool Server::respond(Message& response, IncomingMessage& request)
{
  if (!(request.flags & IncomingMessage::RESPONDED))
  {
    request.flags |= IncomingMessage::RESPONDED;
    return send(response, request.transport, request.client);
  }
  else
  {
    Log::Error << "Duplicate reponse to " << request.transport->name 
	       << ":" << request.client << ":\n";
    Log::Error << response << endl;
    Log::Error << "Original request was:\n";
    Log::Error << request.message << endl;
    return false;
  }
}

//------------------------------------------------------------------------
// Return OK to request given
// Returns whether successul
bool Server::respond(IncomingMessage& request)
{
  OKMessage okm(request.message.get_id());
  return respond(okm, request);
}

//------------------------------------------------------------------------
// Return an error to request given
// Returns whether successul
bool Server::respond(ErrorMessage::Severity severity,
		     const string& text,
		     IncomingMessage& request)
{
  ErrorMessage errm(request.message.get_id(), severity, text);
  return respond(errm, request);
}

//------------------------------------------------------------------------
// Show an incoming message to the correlator before forwarding it,
// in preparation for handling responses - may modify the message
// Note transport and client here are _outgoing_ client you are forwarding to
void Server::correlate(IncomingMessage &msg, Transport *transport,
		       Net::EndPoint& client)
{
  // Check if response required, and if so, correlate it
  if (msg.message.get_rsvp()) 
    correlator.handle_request(msg, transport, client);
}

//------------------------------------------------------------------------
// Destructor
Server::~Server()
{
  // Delete all attached services
  for(list<Service *>::iterator p=services.begin();
      p!=services.end();
      p++)
    delete *p;

  // Delete all attached transports
  for(list<Transport *>::iterator p=transports.begin();
      p!=transports.end();
      p++)
    delete *p;
}

}} // namespaces




