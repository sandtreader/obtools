//==========================================================================
// ObTools::Access: ot-access.h
//
// Public definitions for ObTools::Access
// Generic access checking (ACL) library
//
// Copyright (c) 2008 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_ACCESS_H
#define __OBTOOLS_ACCESS_H

#include "ot-xml.h"
#include "ot-net.h"
#include "ot-ssl.h"
#include <list>
#include <map>

namespace ObTools { namespace Access {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Access group class (group.cc)
class Group
{
 private:
  string id;               // Group ID
  list<string> users;      // List of user name patterns

 public:
  //------------------------------------------------------------------------
  // Constructor - reads from a <group> element
  // ns gives optional namespace prefix for <user> element
  Group(const XML::Element& group_e, const string& ns="");

  //------------------------------------------------------------------------
  // Get ID
  string get_id() { return id; }

  //------------------------------------------------------------------------
  // Check if a given user name is in the group
  bool contains(const string& user);

  //------------------------------------------------------------------------
  // Dump the group to the given ostream
  void dump(ostream& sout) const;
};

//--------------------------------------------------------------------------
// Write group to ostream
ostream& operator<<(ostream& sout, const Group& g);

//==========================================================================
// Individual rule (AND of all conditions) (rule.cc)
class Rule
{
  Group *group;                 // Group to match, or 0 if unset
  string user;                  // User glob pattern to match (or * if unset)
  Net::MaskedAddress address;   // Address mask to match (/0 if unset)

 public:
  //------------------------------------------------------------------------
  // Constructor
  Rule(Group *_group, const string& _user, Net::MaskedAddress _address):
    group(_group), user(_user), address(_address) {}

  //------------------------------------------------------------------------
  // Constructor from a rule element (e.g. <allow .../> or <deny .../>)
  // Looks up group in given group list
  Rule(const XML::Element& r_e, map<string, Group *>& groups);

  //------------------------------------------------------------------------
  // Test the rule for match against the given SSL Client details
  bool matches(Net::IPAddress attempted_address,
     const string& attempted_user);

  //------------------------------------------------------------------------
  // Dump the rule to the given ostream
  void dump(ostream& sout) const;
};

//--------------------------------------------------------------------------
// Write rule to ostream
ostream& operator<<(ostream& sout, const Rule& r);

//==========================================================================
// Resource class (resource.cc)
class Resource
{
 private:
  string name;                 // Glob pattern

  // Rules - denied are checked first
  list<Rule> denied;
  list<Rule> allowed;

 public:
  //------------------------------------------------------------------------
  // Constructor - reads from a <resource> element, using groups in given map
  // ns gives optional namespace prefix
  Resource(const XML::Element& resource_e, map<string, Group *>& groups,
           const string& ns="");

  //------------------------------------------------------------------------
  // Check access to a given real resource by a given user
  // Returns whether the resource matches our pattern - if so, writes the
  // access result to result_p
  bool check(const string& resource, Net::IPAddress address,
             const string& user, bool& result_p);

  //------------------------------------------------------------------------
  // Dump the rule to the given ostream
  void dump(ostream& sout) const;
};

//--------------------------------------------------------------------------
// Write resource to ostream
ostream& operator<<(ostream& sout, const Resource& r);

//==========================================================================
// Access checker class (checker.cc)
class Checker
{
private:
  map<string, Group *> groups;         // Groups, keyed by ID
  list<Resource *> resources;          // Resource rules

public:
  //------------------------------------------------------------------------
  // Constructor
  // Takes <access> config level (containing <groups> and <resources>)
  // Also allows optional namespace prefix for all sub-elements
  Checker(const XML::Element& config, const string& ns="");

  //------------------------------------------------------------------------
  // Check access to a given resource by a given user
  bool check(const string& resource, Net::IPAddress address,
             const string& user);

  //------------------------------------------------------------------------
  // Check access to a given resource by a given SSL client
  // Checks using CN as user, or #anonymous if not identified
  bool check(const string& resource, const SSL::ClientDetails& client)
  {
    return check(resource, client.address.host,
                 client.cert_cn.empty()?"#anonymous":client.cert_cn);
  }

  //------------------------------------------------------------------------
  // Dump the checker rules to the given ostream
  void dump(ostream& sout) const;

  //------------------------------------------------------------------------
  // Destructor
  ~Checker();
};



//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_ACCESS_H



