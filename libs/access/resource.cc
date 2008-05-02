//==========================================================================
// ObTools::Access: resource.cc
//
// Access resource structure
// 
// Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-access.h"
#include "ot-log.h"
#include "ot-text.h"

namespace ObTools { namespace Access {

//--------------------------------------------------------------------------
// Constructor - reads from a <resource> element, using groups in given map
// ns gives optional namespace prefix
Resource::Resource(XML::Element& resource_e, 
		   map<string, Group *>& groups,
		   const string& ns)
{
  Log::Streams log;

  name = resource_e["name"];
  default_allow = (resource_e["default"] == "allow");

  // Get allows
  for(XML::Element::iterator p(resource_e.get_children(ns+"allow")); p; ++p)
  {
    XML::Element& a_e = *p;
    if (a_e.has_attr("group"))
    {
      string gid = a_e["group"];
      map<string, Group *>::iterator q = groups.find(gid);
      if (q == groups.end())
	log.error << "No such group '" << gid << "' quoted for resource " 
		  << name << endl;
      else
	allowed_groups.push_back(q->second);
    }

    if (a_e.has_attr("user"))
      allowed_users.push_back(a_e["user"]);
  }

  // Get denies
  for(XML::Element::iterator p(resource_e.get_children(ns+"deny")); p; ++p)
  {
    XML::Element& d_e = *p;
    if (d_e.has_attr("group"))
    {
      string gid = d_e["group"];
      map<string, Group *>::iterator q = groups.find(gid);
      if (q == groups.end())
	log.error << "No such group '" << gid << "' quoted for resource " 
		  << name << endl;
      else
	denied_groups.push_back(q->second);
    }

    if (d_e.has_attr("user"))
      denied_users.push_back(d_e["user"]);
  }
}
  
//--------------------------------------------------------------------------
// Check access to a given real resource by a given user
// Returns whether the resource matches our pattern - if so, writes the
// access result to result_p
bool Resource::check(const string& resource, const string& user,
		     bool& result_p)
{
  if (!Text::pattern_match(name, resource))  // Note:  Cased
    return false;

  // Check denied first - they override anything else
  for(list<string>::iterator p = denied_users.begin(); 
      p!=denied_users.end(); ++p)
  {
    if (Text::pattern_match(*p, user, false))  // Note:  Uncased
    {
      result_p = false;   // Denied!
      return true;
    }
  }

  for(list<Group *>::iterator p = denied_groups.begin(); 
      p!=denied_groups.end(); ++p)
  {
    Group *g = *p;
    if (g->contains(user))
    {
      result_p = false;   // Denied!
      return true;
    }
  }

  // If default is allowed, that's all we have to check
  if (default_allow)
  {
    result_p = true;      // Allowed
    return true;
  }

  // Now check for explicitly allowed
  for(list<string>::iterator p = allowed_users.begin(); 
      p!=allowed_users.end(); ++p)
  {
    if (Text::pattern_match(*p, user, false))  // Note:  Uncased
    {
      result_p = true;    // Allowed
      return true;
    }
  }

  for(list<Group *>::iterator p = allowed_groups.begin(); 
      p!=allowed_groups.end(); ++p)
  {
    Group *g = *p;
    if (g->contains(user))
    {
      result_p = true;    // Allowed
      return true;
    }
  }

  // Not explicitly allowed and default is deny, so...
  result_p = false;       // Denied!
  return true;            // ... but we did match
}

}} // namespaces




