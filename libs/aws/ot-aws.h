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
#include <set>
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
    Web::MIMEHeaders& headers;   // HTTP headers (modifiable)
    bool sign_payload;           // Whether to include payload in sig
    string payload;              // Optional payload to sign

  RequestInfo(const string& _method, const string& _uri,
              const Time::Stamp& _date, Web::MIMEHeaders& _headers,
              bool _sign_payload=false, const string& _payload=""):
    method(_method), uri(_uri), date(_date), headers(_headers),
    sign_payload(_sign_payload), payload(_payload) {}
  };

 private:
  string access_key_id;
  string secret_key;
  string aws_region;
  string aws_service;

  static Misc::PropertyList get_canonical_headers(const RequestInfo& req);

 public:
  //------------------------------------------------------------------------
  // Constructor
  Authenticator(const string& _access_key_id,
                const string& _secret_key,
                const string& _aws_region,
                const string& _aws_service):
    access_key_id(_access_key_id), secret_key(_secret_key),
    aws_region(_aws_region), aws_service(_aws_service) {}

  //--------------------------------------------------------------------------
  // Set region (for redirection after bucket creation)
  void set_region(const string& region) { aws_region = region; }

  //--------------------------------------------------------------------------
  // Add required aws-headers to the headers in the request
  static void add_aws_headers(const RequestInfo& req);

  // === Partial operations exposed for testing ===

  //--------------------------------------------------------------------------
  // Create canonical request for initial signing
  static string create_canonical_request(const RequestInfo& req);

  //--------------------------------------------------------------------------
  // Get string to sign from a canonical request
  static string get_string_to_sign(const string& canonical_request,
                                   const Time::Stamp& date,
                                   const string& scope);

  //--------------------------------------------------------------------------
  // Get a signing key (uses secret_key)
  string get_signing_key(const Time::Stamp& date);

  //--------------------------------------------------------------------------
  // Get the signature for the string
  static string sign(const string& signing_key, const string& string_to_sign);

  //--------------------------------------------------------------------------
  // Get a credential scope string
  string get_scope_string(const Time::Stamp& date);

  //--------------------------------------------------------------------------
  // Get the signature for a request
  string get_signature(const RequestInfo& req);

  //--------------------------------------------------------------------------
  // Get the Authorization headers for a request
  string get_authorization_header(const RequestInfo& req);

  // === Combined operation to actually use ==

  //--------------------------------------------------------------------------
  // Add the necessary aws- and Authorization headers to a request
  // Modifies req.headers
  void sign(const RequestInfo& req);
};

//==========================================================================
// S3 Client
class S3Client
{
  Authenticator authenticator;
  bool persistent{false};
  auto_ptr<Web::HTTPClient> http;
  int max_requests_per_connection{50};  // AWS RSTs at 100
  int requests_this_connection{0};
  string s3_host;
  string user_agent{"ObTools S3 Client"};
  int connection_timeout{10};
  int operation_timeout{10};

 public:
  static constexpr auto default_s3_host = "s3.amazonaws.com";

  //------------------------------------------------------------------------
  // Constructor
  S3Client(const string& _access_key_id,
           const string& _secret_key,
           const string& _aws_region,
           const string& _s3_host = default_s3_host):
    authenticator(_access_key_id, _secret_key, _aws_region, "s3"),
    s3_host(_s3_host) {}

  //--------------------------------------------------------------------------
  // Set region (for redirection after bucket creation)
  void set_region(const string& region) { authenticator.set_region(region); }

  //--------------------------------------------------------------------------
  // Enable persistent HTTP connections
  void enable_persistence() { persistent = true; }

  //--------------------------------------------------------------------------
  // Set user-agent
  void set_user_agent(const string& ua) { user_agent = ua; }

  //--------------------------------------------------------------------------
  // Set timeouts in seconds
  void set_timeouts(int conn_t, int op_t)
  { connection_timeout = conn_t; operation_timeout = op_t; }

  //--------------------------------------------------------------------------
  // Set maximum requests per connection
  void set_max_requests_per_connection(int max)
  { max_requests_per_connection = max; }

  //--------------------------------------------------------------------------
  // Do an HTTP request, with authentication
  bool do_request(Web::HTTPMessage& request, Web::HTTPMessage &response);

  //--------------------------------------------------------------------------
  // Do an HTTP request on the given URL, with string request and response
  bool do_request(const string& method, const Web::URL& url,
                  const Misc::PropertyList& req_headers,
                  const string& req_s, string& resp_s);

  //--------------------------------------------------------------------------
  // Do an HTTP request on the given URL with no request or response (e.g. DELETE)
  bool do_request(const string& method, const Web::URL& url)
  { string r; Misc::PropertyList h; return do_request(method, url, h, "", r); }

  //--------------------------------------------------------------------------
  // Do an XML HTTP request on the given URL.
  // If request is invalid (Element::none), no request body is sent
  bool do_request(const string& method, const Web::URL& url,
                  const Misc::PropertyList& req_headers,
                  const XML::Element& req_xml, XML::Element& resp_xml);

  //--------------------------------------------------------------------------
  // Do an HTTP GET request on the given URL with XML response
  bool do_request(const Web::URL& url,
                  XML::Element& resp_xml)
  { Misc::PropertyList h;
    return do_request("GET", url, h, XML::Element::none, resp_xml); }

  //--------------------------------------------------------------------------
  // Get S3 REST URL for a given bucket (or all buckets if empty) and object
  // (or all objects if empty)
  Web::URL get_url(const string& bucket_name = "",
                   const string& object_key = "");

  //--------------------------------------------------------------------------
  // List all buckets owned by the user
  bool list_all_my_buckets(set<string>& buckets);

  //--------------------------------------------------------------------------
  // List a specific bucket, with optional prefix
  bool list_bucket(const string& bucket_name, set<string>& objects,
                   const string& prefix = "");

  //--------------------------------------------------------------------------
  // Create a bucket
  // acl can be 'public-read' etc., defaults to 'private'
  // Empty region uses default (us-east-1)
  bool create_bucket(const string& bucket_name,
                     const string& acl = "",
                     const string& region="");

  //--------------------------------------------------------------------------
  // Create an object
  // acl can be 'public-read' etc., defaults to 'private'
  bool create_object(const string& bucket_name, const string& object_key,
                     const string& object_data,
                     const string& acl="");

  //--------------------------------------------------------------------------
  // Get an object
  bool get_object(const string& bucket_name, const string& object_key,
                  string& object_data);

  //--------------------------------------------------------------------------
  // Delete an object
  bool delete_object(const string& bucket_name, const string& object_key);

  //--------------------------------------------------------------------------
  // Delete multiple objects
  // Set max_keys_per_request for testing
  bool delete_multiple_objects(const string& bucket_name,
                               set<string>& object_keys,
                               int max_keys_per_request = 1000);

  //--------------------------------------------------------------------------
  // Delete all objects with a given prefix
  bool delete_objects_with_prefix(const string& bucket_name,
                                  const string& prefix);

  //--------------------------------------------------------------------------
  // Empty the bucket, deleting all objects
  bool empty_bucket(const string& bucket_name);

  //--------------------------------------------------------------------------
  // Delete a bucket (must be empty)
  bool delete_bucket(const string& bucket_name);
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_AWS_H

