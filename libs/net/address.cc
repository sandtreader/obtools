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
// Get hostname (reverse lookup), or dotted quad
string IPAddress::get_hostname() const
{
  struct hostent *host;
  uint32_t nbo_addr = htonl(address);
  host = gethostbyaddr((char *)&nbo_addr, 4, AF_INET);

  if (host)
    return host->h_name;
  else
    return inet_ntoa(*(struct in_addr *)&nbo_addr);
}

//--------------------------------------------------------------------------
// Output dotted quad to given stream
void IPAddress::output_dotted_quad(ostream& s) const
{
  uint32_t nbo_addr = htonl(address);
  s << inet_ntoa(*(struct in_addr *)&nbo_addr);
}

//--------------------------------------------------------------------------
// Dotted quad output operator
ostream& operator<<(ostream& s, const IPAddress& ip)
{
  ip.output_dotted_quad(s);
}


//==========================================================================
// End-points

//--------------------------------------------------------------------------
// Output to given stream
void EndPoint::output(ostream& s) const
{
  s << address << ":" << port;
}

//--------------------------------------------------------------------------
// Dotted quad output operator
ostream& operator<<(ostream& s, const EndPoint& ep)
{
  ep.output(s);
}

}} // namespaces



