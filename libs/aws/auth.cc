//==========================================================================
// ObTools::AWS: auth.cc
//
// Generic AWS authenticator
// Split out and partial operations exposed for testing
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-aws.h"
#include <sstream>

namespace ObTools { namespace AWS {

//--------------------------------------------------------------------------
// Create canonical request for initial signing
string Authenticator::create_canonical_request(
                              const string& method,
                              const string& uri,
                              const Time::Stamp& date,
                              const Misc::PropertyList& query,
                              const Misc::PropertyList& headers,
                              const string& payload)
{
  ostringstream oss;
  oss << method << '\n';
  oss << uri << '\n';

  // Query parameters
  oss << Web::URL::encode(query, false) << '\n';  // Space as %20

  // Headers sorted lower-case
  Misc::PropertyList canon_headers;
  for(const auto& p: headers)
    canon_headers.add(Text::tolower(p.first),
                     Text::canonicalise_space(p.second));

  // Add date
  canon_headers.add("x-amz-date", date.iso_minimal()+"Z");

  // Add hash of payload
  const auto& payload_hash = Crypto::SHA256::digest_hex(payload);
  canon_headers.add("x-amz-content-sha256", payload_hash);

  for(const auto& p: canon_headers)
    oss << p.first << ':' << p.second << '\n';
  oss << '\n';  // Blank line not stated in algorithm but is in example

  // Header list
  bool first = true;
  for(const auto& p: canon_headers)
  {
    oss << (first?"":";") << p.first;
    first = false;
  }

  // Payload hash again
  oss << '\n' << payload_hash;

  return oss.str();
}

//--------------------------------------------------------------------------
// Get string to sign from a canonical request
string Authenticator::get_string_to_sign(
                              const string& canonical_request,
                              const Time::Stamp& date,
                              const string& scope)
{
  ostringstream oss;
  oss << "AWS4-HMAC-SHA256\n";
  oss << date.iso_minimal() << "Z\n";
  oss << scope << endl;
  oss << Crypto::SHA256::digest_hex(canonical_request);
  return oss.str();
}

//--------------------------------------------------------------------------
// Get a signing key
string Authenticator::get_signing_key(const Time::Stamp& date,
                                      const string& aws_region,
                                      const string& aws_service)
{
  string key = Crypto::HMACSHA256::sign("AWS4"+secret_key, date.iso_date(0));
  key = Crypto::HMACSHA256::sign(key, aws_region);
  key = Crypto::HMACSHA256::sign(key, aws_service);
  return Crypto::HMACSHA256::sign(key, "aws4_request");
}

//--------------------------------------------------------------------------
// Get the signature for the string
string Authenticator::sign(const string& signing_key,
                           const string& string_to_sign)
{
  return Text::btox(Crypto::HMACSHA256::sign(signing_key, string_to_sign));
}


}} // namespaces
