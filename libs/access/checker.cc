//==========================================================================
// ObTools::Access: ot-access.h
//
// Access checker module
// 
// Copyright (c) 2008 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-access.h"
#include "ot-log.h"
#include "ot-text.h"

namespace ObTools { namespace Access {

//--------------------------------------------------------------------------
// Takes <access> config level (containing <groups> and <resources>)
// Also allows optional namespace prefix for all sub-elements
Checker::Checker(XML::Element& config, const string& ns)
{
  // Read groups
  XML::Element& groups_e = config.get_child(ns+"groups");
  for(XML::Element::iterator p(groups_e.get_children(ns+"group")); p; ++p)
  {
    XML::Element& g_e = *p;
    Group *g = new Group(g_e, ns);
    groups[g->get_id()] = g;
  }

  // Read resources
  XML::Element& resources_e = config.get_child(ns+"resources");
  for(XML::Element::iterator p(resources_e.get_children(ns+"resource")); p;++p)
  {
    XML::Element& r_e = *p;
    Resource *r = new Resource(r_e, groups, ns);
    resources.push_back(r);
  }
}

//--------------------------------------------------------------------------
// Check access to a given resource by a given user
bool Checker::check(const string& resource, Net::IPAddress address,
		    const string& user)
{
  bool result;  
 
  // Ask all resources;  first one that matches the resource gets to choose
  for(list<Resource *>::iterator p = resources.begin(); p!=resources.end();++p)
  {
    Resource *r = *p;
    if (r->check(resource, address, user, result)) return result;
  }

  return false;  // Fail safe if nothing matches
}

//--------------------------------------------------------------------------
// Destructor
Checker::~Checker()
{
  for(list<Resource *>::iterator p = resources.begin(); p!=resources.end();++p)
    delete *p;

  for(map<string, Group *>::iterator p = groups.begin(); p!=groups.end(); ++p)
    delete p->second;
}


}} // namespaces




