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
 public:
  struct RequestInfo
  {
    string method;               // GET, POST etc.
    string uri;                  // Path part of URI - e.g. /index.html
    Time::Stamp date;            // Time of request
    Misc::PropertyList query;    // Query parameters
    Misc::PropertyList headers;  // HTTP headers
    string *payload{nullptr};    // Optional payload to sign
    string aws_region;           // AWS Region (e.g. us-east-1)
    string aws_service;          // AWS Service (e.g. s3)
  };

 private:
  string access_key_id;
  string secret_key;

  Misc::PropertyList get_canonical_headers(const RequestInfo& req);

 public:
  //------------------------------------------------------------------------
  // Constructor
  Authenticator(const string& _access_key_id,
                const string& _secret_key):
    access_key_id(_access_key_id), secret_key(_secret_key) {}

  // === Partial operations exposed for testing ===

  //--------------------------------------------------------------------------
  // Create canonical request for initial signing
  string create_canonical_request(const RequestInfo& req);

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

  //--------------------------------------------------------------------------
  // Get a credential scope string
  string get_scope(const RequestInfo& req);

  // === Combined signature operations ===
  //--------------------------------------------------------------------------
  // Get the signature for a request
  string get_signature(const RequestInfo& req);

  //--------------------------------------------------------------------------
  // Get the Authorization headers for a request
  string get_authorization_header(const RequestInfo& req);
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

