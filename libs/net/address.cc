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

//--------------------------------------------------------------------------
// Name lookup constructor
IP_Address::IP_Address(const char *hostname)
{
  if (isdigit(*hostname))
  {
    address = inet_addr(hostname);
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
// Reverse lookup
string IP_Address::get_hostname() const
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
// Dotted quad output
void IP_Address::output_dotted_quad(ostream& s) const
{
  uint32_t nbo_addr = htonl(address);
  s << inet_ntoa(*(struct in_addr *)&nbo_addr);
}

//--------------------------------------------------------------------------
// Dotted quad output operator
ostream& operator<<(ostream& s, const IP_Address& e)
{
  e.output_dotted_quad(s);
}


}} // namespaces



