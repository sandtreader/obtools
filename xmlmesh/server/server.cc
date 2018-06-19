//==========================================================================
// ObTools::XMLMesh:Server server.cc
//
// Implementation of XMLMesh server object
//
// Copyright (c) 2003-2015 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "server.h"
#include "ot-log.h"
#include <time.h>

namespace ObTools { namespace XMLMesh {

//--------------------------------------------------------------------------
// Load modules etc. from XML config
void Server::configure(const XML::Configuration& config)
{
  Log::Error log;
  const XML::Element& root = config.get_root();

  // Read all services
  const XML::Element& service_es = root.get_child("services");
  for(XML::Element::iterator p(service_es.children); p; ++p)
  {
    if (!create_service(*p))
      log << "Failed to create service from XML:\n" << *p;
  }

  // Read all routes
  const XML::Element& routes = root.get_child("routes");
  for(XML::Element::const_iterator p(routes.get_children("route")); p; ++p)
  {
    if (!create_route(*p))
      log << "Failed to create route from XML:\n" << *p;
  }
}

//--------------------------------------------------------------------------
// Create a new service from the given XML
// Returns whether successful
bool Server::create_service(const XML::Element& xml)
{
  Service *s = server.service_registry.create(xml.name, xml);
  if (!s) return false;

  // Store it
  services.push_back(s);

  // Map its id, only if it has one
  const string id = xml.get_attr("id");
  if (id.size()) service_ids[id] = s;

  return true;
}

//--------------------------------------------------------------------------
// Create a new route from the given XML
// Returns whether successful
bool Server::create_route(const XML::Element& xml)
{
  Log::Streams log;
  const string from = xml["from"];
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

  const string to = xml["to"];
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

  const string subject = xml.get_attr("subject", "*");

  // Add route to 'from' side
  from_s->add_route(subject, *to_s);

  log.summary << "Created route from '" << from << "' to '" << to
              << "' for subjects '" << subject << "'\n";
  return true;
}

//--------------------------------------------------------------------------
// Look up a service by name
Service *Server::lookup_service(const string& name) const
{
  map<string, Service *>::const_iterator p = service_ids.find(name);
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
    this_thread::sleep_for(chrono::seconds{1});

    // Tick all services
    for(list<Service *>::iterator p=services.begin();
        p!=services.end();
        p++)
    {
      Service *s = *p;
      if (s->started())
        s->tick();
      else
      {
        Log::Error log;
        log << "Service " << s->get_id()
            << " failed to start - shutting down\n";
        return;
      }
    }
  }
}

//--------------------------------------------------------------------------
// Clean shutdown
void Server::shutdown()
{
  // Shutdown all attached services
  for(auto s: services) s->shutdown();
}

//--------------------------------------------------------------------------
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




