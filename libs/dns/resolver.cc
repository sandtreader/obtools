//==========================================================================
// ObTools::DNS: resolver.cc
//
// Portability wrapper around generic DNS resolver
//
// Copyright (c) 2008 Obtools Limited.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-dns.h"
#include "ot-log.h"
#include "ot-chan.h"

#if defined(__WIN32__)
#include "windows.h"
#include "windns.h"
#else
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#endif

#define MAX_RESULT 8192

namespace ObTools { namespace DNS {

//--------------------------------------------------------------------------
// Default constructor
Resolver::Resolver()
{


}


#if !defined(__WIN32__)
//--------------------------------------------------------------------------
// Skip a DNS 'name'
static void skip_name(Channel::Reader& r)
{
  // Loop until zero length
  for(;;)
  {
    // Get length / pointer
    unsigned char len = r.read_byte();
    if (!len) break;

    if ((len & 0xc0) == 0xc0) // 11xxxxxx = pointer
    {
      r.skip(1);   // Skip second byte of pointer
      break;       // and stop
    }
    else
      r.skip(len); // Skip rest of name and continue
  }
}
#endif

//--------------------------------------------------------------------------
// Query for a domain RR of the given type and return raw RDATA from the 
// first answer section
string Resolver::query(const string& domain, Type type,
		       const string& type_name)
{
  Log::Streams log;
  log.detail << "DNS resolver: " << domain << " (" << type_name << ")\n";

#if defined(__WIN32__)
  PDNS_RECORD rr;
  DNS_STATUS status = DnsQuery(domain.c_str(), type, DNS_QUERY_STANDARD, 
			       0, &rr, 0); 

  if (status != NO_ERROR)
  {
    log.error << "DNS resolver: lookup of " << domain << " (" << type_name
	      << ") failed: status=" << status << endl;
    return "";
  }

  // Scan for the right type RR
  for(; rr && rr->wType!=type; rr = rr->pNext)
    ;

  if (!rr)
  {
    // Nothing returned
    log.error << "DNS resolver: lookup of " << domain << " (" << type_name
	      << ") failed: no result\n";
    return "";
  }

  string rdata;

  // This is really silly, but our interface is to return raw RDATA, so
  // we need to undo the processing DnsQuery has already done to unpack it!
  // This saves the platform differences leaking out into the type-specific
  // code below
  switch (type)
  {
    case TYPE_TXT:
      for(unsigned int i=0; i<rr->Data.TXT.dwStringCount; i++)
      {
	char *s = rr->Data.TXT.pStringArray[i];
	// Append Pascal string
	rdata += (char)strlen(s);
	rdata += s;
      }
      break;

    default:  // Anything not understood by DnsQuery - e.g. CERT
      // Use it as is
      rdata.append((char *)(&rr->Data), rr->wDataLength);
  }

  // Tidy up
  DnsRecordListFree(rr, DnsFreeRecordList);

  return rdata;
#else
  // Use libresolv
  unsigned char buf[MAX_RESULT];
  int len = res_query(domain.c_str(), ns_c_in, static_cast<ns_type>(type),
                      buf, MAX_RESULT);
  if (len < 0) 
  {
    log.error << "DNS resolver: lookup of " << domain << " (" << type_name
	      << ") failed\n";
    return "";
  }

  // Parse the result - we get back the full RFC1035 message
  Channel::BlockReader br(buf, len);

  try
  {
    // First the header
    br.skip(2);  // ID
    uint16_t flags = br.read_nbo_16();

    int rcode = flags & 15;
    if (rcode)
    {
      log.error << "DNS resolver: lookup of " << domain << " (" << type_name
		<< ") failed: RCODE=" << rcode << endl;
      return "";
    }

    uint16_t qdcount = br.read_nbo_16();
    uint16_t ancount = br.read_nbo_16();
    br.skip(4);  // NSCOUNT, ARCOUNT not needed

    // Then the query sections, if reflected
    for(int i=0; i<qdcount; i++)
    {
      skip_name(br); // QNAME
      br.skip(4);  // QTYPE and QCLASS
    }

    // Now check each answer RR for the right type
    for(int i=0; i<ancount; i++)
    {
      skip_name(br);  // Name
      uint16_t atype = br.read_nbo_16();
      br.skip(6);     // CLASS and TTL
      uint16_t rdlen = br.read_nbo_16();  // RDLENGTH

      if (atype == type)
      {
	string rdata;
	br.read(rdata, rdlen);  // RDATA
	return rdata;
      }
      else 
      {
	br.skip(rdlen);  // Skip it
      }
    }

    // Nothing matched
    log.error << "DNS resolver: lookup of " << domain << " (" << type_name
	      << ") failed - no suitable answer sections\n";
    return "";
  }
  catch (Channel::Error ce)
  {
    log.error << "DNS resolver: lookup of " << domain << " (" << type_name
	      << ") failed - can't parse response\n";
    return "";
  }
#endif
}

//--------------------------------------------------------------------------
// Query for a TXT record
// Returns repacked TXT data, or "" if it fails
string Resolver::query_txt(const string& domain)
{
  string rdata = query(domain, TYPE_TXT, "TXT");
  unsigned int rdlen = rdata.size();
  if (!rdlen) return "";

  // Parse the result - this is essentially a set of Pascal strings
  // concatenated together
  Channel::StringReader sr(rdata);

  try
  {
    string result;

    while (sr.get_offset() < rdlen)
    {
      unsigned char len = sr.read_byte();
      sr.read(result, len);
      result += "\n";
    }

    return result;
  }
  catch (Channel::Error ce)
  {
    Log::Streams log;
    log.error << "DNS resolver: lookup of " << domain 
	      << " (TXT) failed - bad string lengths in TXT RR\n";
    return "";
  }
}

//--------------------------------------------------------------------------
// Query for a CERT record
// Returns DER format certificate data, or "" if it fails
string Resolver::query_cert(const string& domain)
{
  string rdata = query(domain, TYPE_CERT, "CERT");
  unsigned int rdlen = rdata.size();
  if (!rdlen) return "";

  // Parse the result
  Channel::StringReader sr(rdata);

  try
  {
    // Read the type - must be PKIX/X.509
    int type = sr.read_nbo_16();
    if (type != 1)
    {
      Log::Streams log;
      log.error << "DNS resolver: lookup of " << domain 
		<< " (CERT) failed - not PKIX\n";
      return "";
    }

    // Skip key tag/algorithm
    sr.skip(3);

    // Read DER certificate from the rest
    string der;
    sr.read(der, rdlen-sr.get_offset());
    return der;
  }
  catch (Channel::Error ce)
  {
    Log::Streams log;
    log.error << "DNS resolver: lookup of " << domain 
	      << " (CERT) failed - bad RR format\n";
    return "";
  }
}

}} // namespaces
