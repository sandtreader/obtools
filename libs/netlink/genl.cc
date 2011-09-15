//==========================================================================
// ObTools::Netlink: genl.cc
//
// C++ wrapper for Generic Netlink
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-netlink.h"

namespace ObTools { namespace Netlink {

//========================================================================
// Generic Request

//------------------------------------------------------------------------
// Constructor
GenericRequest::GenericRequest(const GenericNetlink& netlink,
                               uint8_t command, uint8_t version,
                               int flags):
  msg(0)
{
  msg = nlmsg_alloc();
  genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, netlink.get_family(), 0, flags,
              command, version);
}

//------------------------------------------------------------------------
// Set attributes
void GenericRequest::set_string(int attr, const string& s)
{
  if (msg)
    nla_put_string(msg, attr, s.c_str());
}

void GenericRequest::set_uint32(int attr, const uint32_t u)
{
  if (msg)
    nla_put_u32(msg, attr, u);
}

void GenericRequest::set_uint16(int attr, const uint16_t u)
{
  if (msg)
    nla_put_u16(msg, attr, u);
}

void GenericRequest::set_buffer(int attr, void *buff, ssize_t len)
{
  if (msg)
    nla_put(msg, attr, len, buff);
}

bool GenericRequest::begin_nest(int attr)
{
  struct nlattr *nest = nla_nest_start(msg, attr);
  if (!nest)
    return false;
  nesting.push(nest);
  return true;
}

void GenericRequest::end_nest()
{
  if (nesting.size())
  {
    nla_nest_end(msg, nesting.top());
    nesting.pop();
  }
}

//------------------------------------------------------------------------
// Destructor
GenericRequest::~GenericRequest()
{
  if (msg)
  {
    nlmsg_free(msg);
    msg = 0;
  }
}

//------------------------------------------------------------------------
// Message callback hookup
int generic_netlink_callback(struct nl_msg *msg, void *arg)
{
  GenericRequest *request = reinterpret_cast<GenericRequest *>(arg);
  GenericResponse response(msg, request->get_attribute_count(),
                           request->get_policy());
  if (response.error())
    return response.error();

  return request->callback(response);
}

//========================================================================
// Generic Response

//------------------------------------------------------------------------
// Constructor
GenericResponse::GenericResponse(struct nl_msg *_msg, int attr_max,
                                 struct nla_policy *policy):
  msg(_msg), nlh(0), default_attrs(attr_max + 1), parse_error(0)
{
  nlh = nlmsg_hdr(msg);
  if (policy)
    parse_error = genlmsg_parse(nlh, 0, &default_attrs[0], attr_max, policy);
}

//------------------------------------------------------------------------
// Fetch attributes
uint16_t GenericResponse::get_uint16(int attr,
                                     const vector<struct nlattr *>& attrs)
{
  if (!attrs[attr])
    return 0;

  return nla_get_u16(attrs[attr]);
}

uint32_t GenericResponse::get_uint32(int attr,
                                     const vector<struct nlattr *>& attrs)
{
  if (!attrs[attr])
    return 0;

  return nla_get_u32(attrs[attr]);
}

string GenericResponse::get_string(int attr,
                                   const vector<struct nlattr *>& attrs)
{
  if (!attrs[attr])
    return "";

  return nla_get_string(attrs[attr]);
}

bool GenericResponse::get_data(int attr, void *buffer, ssize_t len,
                               const vector<struct nlattr *>& attrs)
{
  if (!attrs[attr])
    return false;

  memcpy(buffer, nla_data(attrs[attr]), len);
  return true;
}

bool GenericResponse::get_nested_attrs(int attr,
                                       const vector<struct nlattr *>& attrs,
                                       vector<struct nlattr *>& nested_attrs,
                                       int attrs_max,
                                       struct nla_policy *policy)
{
  nested_attrs.resize(attrs_max + 1);
  if (nla_parse_nested(&nested_attrs[0], attrs_max, attrs[attr], policy))
    return false;

  return true;
}

//------------------------------------------------------------------------
// Destructor
GenericResponse::~GenericResponse()
{
}

//========================================================================
// Generic Netlink

//------------------------------------------------------------------------
// Constructor
GenericNetlink::GenericNetlink(const string& _family)
{
  socket = nl_handle_alloc();

  if (!socket)
    return;

  genl_connect(socket);

  family = genl_ctrl_resolve(socket, _family.c_str());

  if (family < 0)
  {
    nl_handle_destroy(socket);
    socket = 0;
  }
}

//------------------------------------------------------------------------
// Make request
bool GenericNetlink::send(GenericRequest &request)
{
  if (nl_socket_modify_cb(socket, NL_CB_VALID, NL_CB_CUSTOM,
                          generic_netlink_callback, &request))
    return false;
  nl_send_auto_complete(socket, request.msg);
  int result = nl_recvmsgs_default(socket);
  if (result)
    return false;
  return true;
}

//------------------------------------------------------------------------
// Get last netlink error
const char* GenericNetlink::get_last_error()
{
  return nl_geterror();
}

//------------------------------------------------------------------------
// Destructor
GenericNetlink::~GenericNetlink()
{
  if (socket)
  {
    nl_handle_destroy(socket);
    socket = 0;
  }
}

}} // namespaces
