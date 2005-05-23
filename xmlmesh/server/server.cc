//==========================================================================
// ObTools::XMLMesh:Server server.cc
//
// Implementation of XMLMesh server object
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "server.h"
#include "ot-log.h"
#include <time.h>

namespace ObTools { namespace XMLMesh { 

//------------------------------------------------------------------------
// Load modules etc. from XML config
void Server::configure(XML::Configuration& config)
{
  XML::Element& root = config.get_root();

  // Read all services
  XML::Element& services = root.get_child("services");
  
  OBTOOLS_XML_FOREACH_CHILD(se, services)
    if (!create_service(se))
      log.error << "Failed to create service from XML:\n" << se;
  OBTOOLS_XML_ENDFOR

  // Read all routes
  XML::Element& routes = root.get_child("routes");

  OBTOOLS_XML_FOREACH_CHILD_WITH_TAG(re, routes, "route")
    if (!create_route(re))
      log.error << "Failed to create route from XML:\n" << re;
  OBTOOLS_XML_ENDFOR
}

//------------------------------------------------------------------------
// Create a new service from the given XML
// Returns whether successful
bool Server::create_service(XML::Element& xml)
{
  Service *s = server.service_registry.create(xml.name, xml);
  if (!s) return false;
  
  // Store it
  services.push_back(s);

  // Map its id, only if it has one
  string id = xml.get_attr("id");
  if (id.size()) service_ids[id] = s;

  return true;
}

//------------------------------------------------------------------------
// Create a new route from the given XML
// Returns whether successful
bool Server::create_route(XML::Element& xml)
{
  string from = xml["from"];
  if (!from.size())
  {
    log.error << "No 'from' quoted in route\n";
    return false;
  }

  Service *from_s = lookup_service(from);
  if (!from_s)
  {
    log.error << "No such 'from' service in route: '" << from << "'\n";
    return false;
  }

  string to = xml["to"];
  if (!to.size())
  {
    log.error << "No 'to' quoted in route\n";
    return false;
  }

  Service *to_s = lookup_service(to);
  if (!to_s)
  {
    log.error << "No such 'to' service in route: '" << to << "'\n";
    return false;
  }

  string subject = xml.get_attr("subject", "*");

  // Add route to 'from' side
  from_s->add_route(subject, *to_s);

  log.summary << "Created route from '" << from << "' to '" << to
	      << "' for subjects '" << subject << "'\n";
  return true;
}

//------------------------------------------------------------------------
// Look up a service by name 
Service *Server::lookup_service(const string& name)
{
  map<string, Service *>::iterator p = service_ids.find(name);
  if (p!=service_ids.end())
    return p->second;
  else
    return 0;
}

//--------------------------------------------------------------------------
// Server run method
void Server::run() 
{ 
  for(;;)
  {
    sleep(1); 

    // Tick all services 
    for(list<Service *>::iterator p=services.begin();
	p!=services.end();
	p++)
      (*p)->tick();
  }
}

//------------------------------------------------------------------------
// Signal all services a global event (e.g. clients starting and finishing)
void Server::signal_services(Service::Signal sig, ServiceClient& client)
{
  for(list<Service *>::iterator p=services.begin();
      p!=services.end();
      p++)
    (*p)->signal(sig, client);
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
}

}} // namespaces




