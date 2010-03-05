//==========================================================================
// ObTools::Access: rule.cc
//
// Access rule (AND of individual conditions)
// 
// Copyright (c) 2008 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-access.h"
#include "ot-log.h"
#include "ot-text.h"

namespace ObTools { namespace Access {

//--------------------------------------------------------------------------
// Constructor from a rule element (e.g. <allow .../> or <deny .../>)
// Looks up group in given group list
Rule::Rule(const XML::Element& r_e, map<string, Group *>& groups): group(0)
{
  // Look for group
  if (r_e.has_attr("group"))
  {
    string gid = r_e["group"];
    map<string, Group *>::iterator q = groups.find(gid);
    if (q == groups.end())
    {
      Log::Streams log;
      log.error << "No such group '" << gid << "' in access rule\n";
    }
    else group = q->second;
  }
 
  // Look for user, default to *
  user = r_e.get_attr("user", "*");

  // Get MaskedAddress
  if (r_e.has_attr("address"))
    address = Net::MaskedAddress(r_e["address"]);
  else
    address = Net::MaskedAddress("0.0.0.0/0");  // Anything
}

//--------------------------------------------------------------------------
// Test the rule for match against the given address and username
bool Rule::matches(Net::IPAddress attempted_address, 
		   const string& attempted_user)
{
  // Note: All specified conditions must match
  
  // Check group (if set)
  if (group && !group->contains(attempted_user)) 
    return false;

  // Check user (set to * if unspecified, so no need to check here)
  if (!Text::pattern_match(user, attempted_user, false))  // Note:  Uncased
    return false;

  // Check address
  return address == attempted_address;  // MaskedAddress compare
}

//--------------------------------------------------------------------------
// Dump the rule to the given ostream
void Rule::dump(ostream& sout) const
{
  if (group) sout << " group '" << group->get_id() << "'";
  if (user != "*") sout << " user '" << user << "'";
  if (address.get_network_bits()) 
    sout << " address " << address;
  else if (!group && user=="*")
    sout << " all";
}

//--------------------------------------------------------------------------
// Write rule to ostream
ostream& operator<<(ostream& sout, const Rule& r)
{ 
  r.dump(sout); 
  return sout; 
}


}} // namespaces




