//==========================================================================
// ObTools::DNS: ot-dns.h
//
// Public definitions for ObTools::DNS
// Portable DNS lookups for TXT, CERT, MX etc. records which can't be got
// from gethostbyname (or ot-net.h wrapper)
//
// Copyright (c) 2008 Obtools Limited.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_DNS_H
#define __OBTOOLS_DNS_H

#include <string>

namespace ObTools { namespace DNS {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Query type
// Values match DNS RR type values (and those in arpa/nameser.h)
enum Type
{
  TYPE_TXT = 16,
  TYPE_CERT = 37
};

//==========================================================================
// Resolver class
class Resolver
{
public:
  //------------------------------------------------------------------------
  // Default constructor
  Resolver();

  //------------------------------------------------------------------------
  // Query for a domain RR of the given type and return raw RDATA from the
  // first answer section
  string query(const string& domain, Type type, const string& type_name="");

  //------------------------------------------------------------------------
  // Query for a TXT record
  // Returns repacked TXT data, or "" if it fails
  string query_txt(const string& domain);

  //------------------------------------------------------------------------
  // Query for a CERT record
  // Returns DER format (binary) certificate data, or "" if it fails
  string query_cert(const string& domain);
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_DNS_H
















