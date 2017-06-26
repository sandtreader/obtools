//==========================================================================
// ObTools::AWS: ot-aws.h
//
// Public definitions for ObTools::AWS
// Client library for Amazon Web Services, initially S3
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_AWS_H
#define __OBTOOLS_AWS_H

#include <string>
#include "ot-crypto.h"
#include "ot-web.h"

namespace ObTools { namespace AWS {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// AWS Authenticator
class Authenticator
{
  string access_key_id;
  string secret_key;

 public:
  //------------------------------------------------------------------------
  // Constructor
  Authenticator(const string& _access_key_id,
                const string& _secret_key):
    access_key_id(_access_key_id), secret_key(_secret_key) {}

  // === Partial operations exposed for testing ===

  //--------------------------------------------------------------------------
  // Create canonical request for initial signing
  string create_canonical_request(const string& method,
                                  const string& uri,
                                  const Time::Stamp& date,
                                  const Misc::PropertyList& query,
                                  const Misc::PropertyList& headers,
                                  const string& payload);

  //--------------------------------------------------------------------------
  // Get string to sign from a canonical request
  string get_string_to_sign(const string& canonical_request,
                            const Time::Stamp& date,
                            const string& scope);

  //--------------------------------------------------------------------------
  // Get a signing key
  string get_signing_key(const Time::Stamp& date,
                         const string& aws_region,
                         const string& aws_service);

  //--------------------------------------------------------------------------
  // Get the signature for the string
  string sign(const string& signing_key, const string& string_to_sign);

  // === Combined signature operation ===
  //--------------------------------------------------------------------------
  // Get the signature for a request
  string get_signature(const string& method,
                       const string& uri,
                       const Time::Stamp& date,
                       const Misc::PropertyList& query,
                       const Misc::PropertyList& headers,
                       const string& payload,
                       const string& aws_region,
                       const string& aws_service);

};

//==========================================================================
// S3 Client
class S3Client
{
  Authenticator authenticator;
  string s3_host;

 public:
  static constexpr auto default_s3_host = "s3.amazonaws.com";

  //------------------------------------------------------------------------
  // Constructor
  S3Client(const string& _access_key_id,
           const string& _secret_key,
           const string& _s3_host = default_s3_host):
    authenticator(_access_key_id, _secret_key), s3_host(_s3_host) {}
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_AWS_H

