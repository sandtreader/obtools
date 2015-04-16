//==========================================================================
// ObTools::Access: group.cc
//
// Access group structure
// 
// Copyright (c) 2008 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-access.h"
#include "ot-log.h"
#include "ot-text.h"

namespace ObTools { namespace Access {

//--------------------------------------------------------------------------
// Constructor - reads from a <group> element
// ns gives optional namespace prefix for <user> element
Group::Group(const XML::Element& group_e, const string& ns)
{
  id = group_e["id"];

  for(XML::Element::const_iterator p(group_e.get_children(ns+"user")); p; ++p)
  {
    const XML::Element& u_e = *p;
    users.push_back(u_e["name"]);
  }
}
  
//--------------------------------------------------------------------------
// Check if a given user name is in the group
bool Group::contains(const string& user)
{
  for(list<string>::iterator p = users.begin(); p!=users.end(); ++p)
  {
    const string& pattern = *p;
    // Note: *Uncased* 'glob' pattern match
    if (Text::pattern_match(pattern, user, false))
      return true;
  }

  return false;
}

//--------------------------------------------------------------------------
// Dump the group to the given ostream
void Group::dump(ostream& sout) const
{
  sout << "Group " << id << ":\n";

  for(list<string>::const_iterator p = users.begin(); p!=users.end(); ++p)
    sout << "    user " << *p << endl;
}

//--------------------------------------------------------------------------
// Write group to ostream
ostream& operator<<(ostream& sout, const Group& g)
{ 
  g.dump(sout); 
  return sout; 
}


}} // namespaces




