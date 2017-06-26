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
// Add required aws-headers to the given headers
void Authenticator::add_aws_headers(const RequestInfo& req)
{
  // Add date
  req.headers.put("x-amz-date", req.date.iso_minimal()+"Z");

  // Add payload hash if required
  if (req.sign_payload)
  {
    const auto& payload_hash = Crypto::SHA256::digest_hex(req.payload);
    req.headers.put("x-amz-content-sha256", payload_hash);
  }
  else
  {
    req.headers.put("x-amz-content-sha256", "UNSIGNED-PAYLOAD");
  }
}

//--------------------------------------------------------------------------
// Create canonical header list
Misc::PropertyList Authenticator::get_canonical_headers(const RequestInfo& req)
{
  Misc::PropertyList canon_headers;
  for(const auto& p: req.headers.xml.children)
    canon_headers.add(Text::tolower(p->name),
                      Text::canonicalise_space(**p));
  return canon_headers;
}

//--------------------------------------------------------------------------
// Create canonical request for initial signing
string Authenticator::create_canonical_request(const RequestInfo& req)
{
  ostringstream oss;
  oss << req.method << '\n';
  oss << req.uri << '\n';

  // Query parameters
  oss << Web::URL::encode(req.query, false) << '\n';  // Space as %20

  // Headers sorted lower-case
  Misc::PropertyList canon_headers = get_canonical_headers(req);
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
  oss << '\n' << canon_headers["x-amz-content-sha256"];

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
                                      const CredentialScope& scope)
{
  string key = Crypto::HMACSHA256::sign("AWS4"+secret_key, date.iso_date(0));
  key = Crypto::HMACSHA256::sign(key, scope.region);
  key = Crypto::HMACSHA256::sign(key, scope.service);
  return Crypto::HMACSHA256::sign(key, "aws4_request");
}

//--------------------------------------------------------------------------
// Get the signature for the string
string Authenticator::sign(const string& signing_key,
                           const string& string_to_sign)
{
  return Text::btox(Crypto::HMACSHA256::sign(signing_key, string_to_sign));
}

//--------------------------------------------------------------------------
// Get a credential scope string
string Authenticator::get_scope_string(const RequestInfo& req)
{
  return req.date.iso_date(0)+"/"+req.scope.region+"/"
        +req.scope.service+"/aws4_request";
}

//--------------------------------------------------------------------------
// Get the signature for a request
string Authenticator::get_signature(const RequestInfo& req)
{
  string canon_request = create_canonical_request(req);
  string scope = get_scope_string(req);
  string sts = get_string_to_sign(canon_request, req.date, scope);
  string key = get_signing_key(req.date, req.scope);
  return sign(key, sts);
}

//--------------------------------------------------------------------------
// Get the Authorization headers for a request
string Authenticator::get_authorization_header(const RequestInfo& req)
{
  ostringstream oss;
  oss << "AWS4-HMAC-SHA256 Credential=";
  oss << access_key_id << '/' << get_scope_string(req);
  oss << ",SignedHeaders=";

  Misc::PropertyList canon_headers = get_canonical_headers(req);
  bool first = true;
  for(const auto& p: canon_headers)
  {
    oss << (first?"":";") << p.first;
    first = false;
  }

  oss << ",Signature=" << get_signature(req);
  return oss.str();
}

}} // namespaces
