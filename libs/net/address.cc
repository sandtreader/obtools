//==========================================================================
// ObTools::Net: address.cc
//
// Internet addressing
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-net.h"
#include <ctype.h>
#include <sstream>
#include <stdlib.h>

#if !defined(__WIN32__)
#include <netdb.h>
#include <arpa/inet.h>
#endif

// Auxiliary data buffer size for gethostbyname_r
#define AUX_BUF_SIZE 1024

namespace ObTools { namespace Net {

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
#if defined(__WIN32__) || defined(__APPLE__)
    // Believed to be threadsafe (using TLS for result), but doesn't
    // provide gethostbyname_r anyway
    struct hostent *host;
    host = gethostbyname(hostname);

    if (host)
      address = (uint32_t)ntohl(*(unsigned long *)(host->h_addr));
    else
      address = BADADDR;
#else
    // Make sure we use threadsafe version
    struct hostent host;
    struct hostent *result;
    char buf[AUX_BUF_SIZE];
    int err;
    if (!gethostbyname_r(hostname, &host, buf, AUX_BUF_SIZE, &result, &err)
        && result)
      address = static_cast<uint32_t>(
                        ntohl(*reinterpret_cast<unsigned long *>(host.h_addr)));
    else
      address = BADADDR;
#endif
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
  uint32_t nbo_addr = nbo();

#if defined(__WIN32__) || defined(__APPLE__)
  // Believed to be threadsafe (using TLS for result), but doesn't
  // provide gethostbyaddr_r anyway
  struct hostent *host;
  host = gethostbyaddr((char *)&nbo_addr, 4, AF_INET);

  if (host)
    return host->h_name;
  else
    return get_dotted_quad();
#else
  // Use thread-safe version
  struct hostent host;
  struct hostent *result;
  char buf[AUX_BUF_SIZE];
  int err;

  if (!gethostbyaddr_r(reinterpret_cast<char *>(&nbo_addr), 4, AF_INET,
                       &host, buf, AUX_BUF_SIZE, &result, &err)
        && result)
    return host.h_name;
  else
    return get_dotted_quad();
#endif
}

//--------------------------------------------------------------------------
// Dotted quad output operator
ostream& operator<<(ostream& s, const IPAddress& ip)
{
  struct in_addr in;
  in.s_addr = ip.nbo();
  s << inet_ntoa(in);
  return s;
}

//==========================================================================
// Masked addresses

//--------------------------------------------------------------------------
// Constructor from CIDR-form a.b.c.d/xx or a.b.c.d/A.B.C.D
// - e.g. 192.168.1.0/24 or 192.168.1.0/255.255.255.0
MaskedAddress::MaskedAddress(const string& cidr):
  address(), mask()
{
  string::size_type p = cidr.find('/');
  if (p != string::npos)
  {
    string addr_s(cidr, 0, p);
    address = IPAddress(addr_s);

    string mask_s(cidr, p+1);

    // Check for dots
    if (mask_s.find('.') != string::npos)
    {
      // It's DQ
      mask = IPAddress(mask_s);
    }
    else
    {
      // It's a bit count
      int n = atoi(mask_s.c_str());
      uint32_t maskbits;

      // Work out mask bits from number of bits
      if (!n)
        maskbits = 0;
      else
        maskbits = ~((1 << (32-n))- 1);  // Beware: Dark Arts

      mask = IPAddress(maskbits);
    }
  }
  else address = IPAddress(cidr);
}

//--------------------------------------------------------------------------
// Get number of network bits in mask
int MaskedAddress::get_network_bits() const
{
  // Binary chop - no more than 5 operations
  uint32_t a = mask.hbo();
  int n=0;

  if (a & 0xFFFF0000UL) { n+=16; a &= 0xFFFF; }
  if (a & 0xFF00)       { n+= 8; a &= 0xFF;   }
  if (a & 0xF0)         { n+= 4; a &= 0xF;    }
  if (a & 0xC0)         { n+= 2; a &= 3;      }
  if (a & 2)            { n+= 1; a &= 1;      }
  return n+a;
}

//--------------------------------------------------------------------------
// Get CIDR form (full netmask type)
string MaskedAddress::get_cidr() const
{
  ostringstream s;
  s << *this;
  return s.str();
}

//--------------------------------------------------------------------------
// << operator to write MaskedAddress to ostream
// e.g. cout << addr;
// Always outputs in CIDR /N form
ostream& operator<<(ostream& s, const MaskedAddress& ip)
{
  s << ip.address;
  if (!!ip.mask) s << '/' << ip.get_network_bits();
  return s;
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
  return s;
}

//==========================================================================
// Protocol

//--------------------------------------------------------------------------
// Constructor from string
Protocol::Protocol(const string& ps)
{
  if (ps=="TCP" || ps=="tcp")
    proto = PROTO_TCP;
  else if (ps=="UDP" || ps=="udp")
    proto = PROTO_UDP;
  else
    proto = PROTO_UNKNOWN;
}

//--------------------------------------------------------------------------
// Output to ostream
void Protocol::output(ostream& s) const
{
  switch (proto)
  {
    case PROTO_TCP:
      s << "TCP";
      break;

    case PROTO_UDP:
      s << "UDP";
      break;

    default:
      s << "?UKNOWN?";
      break;
  }
}

//--------------------------------------------------------------------------
// << operator to write Protocol to ostream
// e.g. cout << proto;
ostream& operator<<(ostream& s, const Protocol& p)
{
  p.output(s);
  return s;
}

//--------------------------------------------------------------------------
// Standard protocols
Protocol Protocol::TCP(Protocol::PROTO_TCP);
Protocol Protocol::UDP(Protocol::PROTO_UDP);
Protocol Protocol::UNKNOWN(Protocol::PROTO_UNKNOWN);

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
  return s;
}

}} // namespaces



