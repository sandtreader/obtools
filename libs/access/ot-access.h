//==========================================================================
// ObTools::Access: ot-access.h
//
// Public definitions for ObTools::Access
// Generic access checking (ACL) library
// 
// Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_ACCESS_H
#define __OBTOOLS_ACCESS_H

#include "ot-xml.h"
#include "ot-ssl.h"
#include <list>
#include <map>

namespace ObTools { namespace Access { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Access group class (group.cc)
class Group
{
 private:
  string id;               // Group ID
  list<string> users;      // List of user name patterns

 public:
  //--------------------------------------------------------------------------
  // Constructor - reads from a <group> element
  // ns gives optional namespace prefix for <user> element
  Group(XML::Element& group_e, const string& ns="");

  //--------------------------------------------------------------------------
  // Get ID
  string get_id() { return id; }
  
  //--------------------------------------------------------------------------
  // Check if a given user name is in the group
  bool contains(const string& user);
};

//==========================================================================
// Resource rule class (resource.cc)
class Resource 
{
 private:
  string name;                 // Glob pattern

  // Rules on individual users (by pattern match)
  list<string> allowed_users;
  list<string> denied_users;

  // Rules on groups
  list<Group *> allowed_groups;
  list<Group *> denied_groups;

  bool default_allow;

 public:
  //--------------------------------------------------------------------------
  // Constructor - reads from a <resource> element, using groups in given map
  // ns gives optional namespace prefix
  Resource(XML::Element& resource_e, map<string, Group *>& groups,
	   const string& ns="");

  //--------------------------------------------------------------------------
  // Check access to a given real resource by a given user
  // Returns whether the resource matches our pattern - if so, writes the
  // access result to result_p
  bool check(const string& resource, const string& user, bool& result_p);
};

//==========================================================================
// Access checker class (checker.cc)
class Checker
{
private:
  map<string, Group *> groups;         // Groups, keyed by ID
  list<Resource *> resources;          // Resource rules

public:
  //--------------------------------------------------------------------------
  // Constructor 
  // Takes <access> config level (containing <groups> and <resources>)
  // Also allows optional namespace prefix for all sub-elements
  Checker(XML::Element& config, const string& ns="");

  //--------------------------------------------------------------------------
  // Check access to a given resource by a given user
  bool check(const string& resource, const string& user);

  //--------------------------------------------------------------------------
  // Check access to a given resource by a given SSL client
  // Checks using CN as user, or #anonymous if not identified
  // !!! Could check IP address here, too
  bool check(const string& resource, SSL::ClientDetails& client)
  {
    return check(resource, client.cert_cn.empty()?"#anonymous":client.cert_cn);
  }

  //--------------------------------------------------------------------------
  // Destructor
  ~Checker();
};



//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_ACCESS_H



