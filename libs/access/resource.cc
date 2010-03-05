//==========================================================================
// ObTools::Access: resource.cc
//
// Access resource structure
// 
// Copyright (c) 2008 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
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

  // Get allows
  for(XML::Element::iterator p(resource_e.get_children(ns+"allow")); p; ++p)
  {
    XML::Element& a_e = *p;
    allowed.push_back(Rule(a_e, groups));
  }

  // Get denies
  for(XML::Element::iterator p(resource_e.get_children(ns+"deny")); p; ++p)
  {
    XML::Element& d_e = *p;
    denied.push_back(Rule(d_e, groups));
  }
}
  
//--------------------------------------------------------------------------
// Check access to a given real resource by a given user
// Returns whether the resource matches our pattern - if so, writes the
// access result to result_p
bool Resource::check(const string& resource, Net::IPAddress address,
		     const string& user, bool& result_p)
{
  if (!Text::pattern_match(name, resource))  // Note:  Cased
    return false;

  // Check denied first - they override anything else
  for(list<Rule>::iterator p = denied.begin(); p!=denied.end(); ++p)
  {
    Rule& rule = *p;
    if (rule.matches(address, user))
    {
      result_p = false;   // Denied!
      return true;
    }
  }

  // Now check for allowed
  for(list<Rule>::iterator p = allowed.begin(); p!=allowed.end(); ++p)
  {
    Rule& rule = *p;
    if (rule.matches(address, user))
    {
      result_p = true;   // Allowed
      return true;
    }
  }

  // Not explicitly allowed and default is deny, so...
  result_p = false;       // Denied!
  return true;            // ... but we did match
}

//--------------------------------------------------------------------------
// Dump the resource to the given ostream
void Resource::dump(ostream& sout) const
{
  sout << "Resource " << name << ":\n";

  for(list<Rule>::const_iterator p = denied.begin(); p!=denied.end(); ++p)
  {
    const Rule& rule = *p;
    sout << "  - deny" << rule << endl;
  }

  for(list<Rule>::const_iterator p = allowed.begin(); p!=allowed.end(); ++p)
  {
    const Rule& rule = *p;
    sout << "  - allow" << rule << endl;
  }
}

//--------------------------------------------------------------------------
// Write rule to ostream
ostream& operator<<(ostream& sout, const Resource& r)
{ 
  r.dump(sout); 
  return sout; 
}


}} // namespaces




