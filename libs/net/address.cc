//==========================================================================
// ObTools::Net: address.cc
//
// Internet addressing
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-net.h"
#include <ctype.h>
#include <netdb.h>
#include <arpa/inet.h>

namespace ObTools { namespace Net {

//!!! Consider MT issues here - may need global resolver lock

//==========================================================================
// IP Addresses

//--------------------------------------------------------------------------
// Name lookup constructor
IPAddress::IPAddress(const string& hostname_s)
{
  const char *hostname = hostname_s.c_str();
  if (isdigit(*hostname))
  {
    address = ntohl(inet_addr(hostname));
  }
  else
  {
    struct hostent *host;
    host = gethostbyname(hostname);               

    if (host) 
      address = (uint32_t)ntohl(*(unsigned long *)(host->h_addr));
    else
      address = BADADDR;
  }
}

//--------------------------------------------------------------------------
// Get dotted quad
string IPAddress::get_dotted_quad() const
{
  struct in_addr in;
  in.s_addr = nbo();
  return string(inet_ntoa(in));
}

//--------------------------------------------------------------------------
// Get hostname (reverse lookup), or dotted quad
string IPAddress::get_hostname() const
{
  struct hostent *host;
  uint32_t nbo_addr = nbo();
  host = gethostbyaddr((char *)&nbo_addr, 4, AF_INET);

  if (host)
    return host->h_name;
  else
    return get_dotted_quad();
}

//--------------------------------------------------------------------------
// Dotted quad output operator
ostream& operator<<(ostream& s, const IPAddress& ip)
{
  struct in_addr in;
  in.s_addr = ip.nbo();
  s << inet_ntoa(in);
}


//==========================================================================
// End-points

//--------------------------------------------------------------------------
// Output to given stream
void EndPoint::output(ostream& s) const
{
  s << host << ":" << port;
}

//--------------------------------------------------------------------------
// Dotted quad output operator
ostream& operator<<(ostream& s, const EndPoint& ep)
{
  ep.output(s);
}

//==========================================================================
// Protocol

//------------------------------------------------------------------------
// << operator to write Protocol to ostream
// e.g. cout << proto;
ostream& operator<<(ostream& s, const Protocol& p)
{
  s << ((p==TCP)?"TCP":"UDP");
}

//==========================================================================
// Ports

//--------------------------------------------------------------------------
// Output to given stream
void Port::output(ostream& s) const
{
  s << proto << ":" << host << ":" << port;
}

//--------------------------------------------------------------------------
// Dotted quad output operator
ostream& operator<<(ostream& s, const Port& p)
{
  p.output(s);
}

}} // namespaces



