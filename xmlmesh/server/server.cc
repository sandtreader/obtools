//==========================================================================
// ObTools::XMLBus:Server server.cc
//
// Implementation of XMLBus server object
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "server.h"
#include "ot-log.h"

namespace ObTools { namespace XMLBus { 

//------------------------------------------------------------------------
// Constructor
Server::Server()
{

}

//------------------------------------------------------------------------
// Register a transport type
void Server::register_transport(const string& name, 
				ServerTransportFactory *factory)
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
  map<string, ServerTransportFactory *>::iterator p;
  p = transport_factories.find(xml.name);
  if (p==transport_factories.end()) return false;

  // Call factory to create transport
  ServerTransportFactory *factory = p->second;
  ServerTransport *t = factory->create(xml);
  
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
  Service *s = factory->create(xml);
  
  // Store it
  services.push_back(s);

  return true;
}

//--------------------------------------------------------------------------
// Server run method
// Pull messages off receive queue and distributes them
void Server::run() 
{ 
  for(;;)
  {
    // Block for a message
    IncomingMessage msg = incoming_q.wait();

    // Deal with it
    distributor.distribute(msg);
  }
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
  for(list<ServerTransport *>::iterator p=transports.begin();
      p!=transports.end();
      p++)
    delete *p;
}

}} // namespaces




