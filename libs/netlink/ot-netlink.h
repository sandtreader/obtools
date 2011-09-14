//==========================================================================
// ObTools::Netlink: ot-netlink.h
//
// Public definitions for ObTools::Netlink
// C++ wrapping of netlink etc.
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_NETLINK_H
#define __OBTOOLS_NETLINK_H

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <string>
#include <vector>
#include <stack>

namespace ObTools { namespace Netlink {

using namespace std;

class GenericNetlink;
class GenericResponse;

//==========================================================================
// Generic Request

class GenericRequest
{
  friend class GenericNetlink;
private:
  struct nl_msg *msg;
  stack<struct nlattr *> nesting;

public:
  //------------------------------------------------------------------------
  // Constructor
  GenericRequest(const GenericNetlink& netlink,
                 uint8_t command, uint8_t version);

  //------------------------------------------------------------------------
  // Callback for handling response
  virtual int callback(GenericResponse&) { return 0; }

  //------------------------------------------------------------------------
  // Fetch number of attributes in response
  virtual int get_attribute_count() { return 0; }

  //------------------------------------------------------------------------
  // Fetch response parsing policy
  virtual struct nla_policy *get_policy() { return 0; }

  //------------------------------------------------------------------------
  // Set attributes
  void set_string(int attr, const string& s);
  bool begin_nest(int attr);
  void end_nest();

  //------------------------------------------------------------------------
  // Destructor
  virtual ~GenericRequest();
};

//==========================================================================
// Generic Response

class GenericResponse
{
private:
  struct nl_msg *msg;
  struct nlmsghdr *nlh;
  vector<struct nlattr *> default_attrs;
  int parse_error;

public:
  //------------------------------------------------------------------------
  // Constructor
  GenericResponse(struct nl_msg *_msg, int attr_max,
                  struct nla_policy* policy);

  //------------------------------------------------------------------------
  // Fetch attributes
  uint32_t get_uint32(int attr, const vector<struct nlattr *>& attrs);
  uint32_t get_uint32(int attr) { return get_uint32(attr, default_attrs); }
  string get_string(int attr, const vector<struct nlattr *>& attrs);
  string get_string(int attr) { return get_string(attr, default_attrs); }

  // Nested attributes
  bool get_nested_attrs(int attr, const vector<struct nlattr *>& attrs,
                        vector<struct nlattr *>& nested_attrs, int attrs_max,
                        struct nla_policy* policy);
  bool get_nested_attrs(int attr,
                        vector<struct nlattr *>& nested_attrs, int attrs_max,
                        struct nla_policy* policy)
  {
    return get_nested_attrs(attr, default_attrs,
                            nested_attrs, attrs_max,
                            policy);
  }

  //------------------------------------------------------------------------
  // Error value from parsing response
  int error() { return parse_error; }

  //------------------------------------------------------------------------
  // Destructor
  ~GenericResponse();
};

//------------------------------------------------------------------------
// Message callback hookup
int generic_netlink_callback(struct nl_msg *response, void *arg);

//==========================================================================
// Generic Netlink

class GenericNetlink
{
private:
  struct nl_handle *socket;
  int family;

public:
  //------------------------------------------------------------------------
  // Constructor
  GenericNetlink(const string& _family);

  //------------------------------------------------------------------------
  // Test if valid netlink
  bool valid() const { return socket; }

  //------------------------------------------------------------------------
  // Get the family (e.g. for message construction)
  int get_family() const { return family; }

  //------------------------------------------------------------------------
  // Make request
  bool send(GenericRequest &request);

  //------------------------------------------------------------------------
  // Destructor
  ~GenericNetlink();
};

}} // namespaces

#endif
