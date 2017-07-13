//==========================================================================
// ObTools::AWS: s3client.cc
//
// AWS S3 client implementation
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-aws.h"
#include "ot-misc.h"

namespace ObTools { namespace AWS {

//--------------------------------------------------------------------------
// Do an HTTP request, with authentication
bool S3Client::do_request(Web::HTTPMessage& request, Web::HTTPMessage &response)
{
  // Create persistent client if it doesn't already exist
  if (!http.get())
  {
    Log::Detail log;
    log << "S3 creating new HTTP client on " << request.url << endl;
    http.reset(new Web::HTTPClient(request.url, 0, user_agent,
                                   connection_timeout, operation_timeout));
    if (persistent) http->enable_persistence();
    requests_this_connection = 0;
  }

  // Add headers and authenticate
  request.headers.put("host", request.url.get_host());

  // Content-MD5 required for multi-object delete for no apparent reason
  // (body content is signed anyway) - add it for all
  if (!request.body.empty())
  {
    Misc::MD5 md5;
    request.headers.put("Content-MD5", md5.sum_base64(request.body));
  }

  AWS::Authenticator::RequestInfo req(request.method,
                                      request.url.get_path(),
                                      Time::Stamp::now(),
                                      request.headers,
                                      true, request.body);

  // Add query items
  request.url.get_query(req.query);

  // Sign it
  authenticator.sign(req);

  // Check for max requests and close if reached
  bool close = ++requests_this_connection >= max_requests_per_connection;
  if (close) http->close_persistence();

  // Do the fetch
  bool result = http->fetch(request, response);

  // Close down if not persistent
  if (!persistent || close) http.reset();

  return result;
}

//--------------------------------------------------------------------------
// Do an HTTP request on the given URL, with string request and response
bool S3Client::do_request(const string& method,
                          const Web::URL& url,
                          const Misc::PropertyList& req_headers,
                          const string& req_s,
                          string& resp_s)
{
  Log::Streams log;
  Web::URL the_url = url;
  Web::HTTPMessage response;

  // Loop for redirects
  for(int redirect=0; redirect<2; redirect++)
  {
    log.detail << "S3 " << method << " " << the_url << endl;
    Web::HTTPMessage request(method, the_url);
    for(const auto& p: req_headers)
      request.headers.put(p.first, p.second);
    request.body = req_s;
    OBTOOLS_LOG_IF_DEBUG(log.debug << request.body;)

    if (!do_request(request, response)) return false;

    resp_s = response.body;

    // Check for 307 Redirects when switching region, but only allow once
    if (!redirect && response.code == 307)
    {
      // Try to parse it
      XML::Parser parser(log.error);
      try
      {
        parser.read_from(response.body);
      }
      catch (XML::ParseFailed)
      {
        log.error << "Bad XML in S3 response\n";
        return false;
      }

      const string& endpoint = *parser.get_root().get_child("Endpoint");
      if (endpoint.empty()) break;

      log.detail << "S3 307 redirect to " << endpoint << endl;

      // Just substitute into URL
      the_url = Web::URL(Text::subst(the_url.str(), the_url.get_host(),
                                     endpoint));

      // Shut down current HTTP client
      http.reset();
    }
    else break;
  }

  if (response.code/100 != 2) // Allow for 200, 204 etc.
  {
    log.error << "S3 request " << method << " " << url << " failed:\n";
    log.error << response.code << " " << response.reason << endl;
    log.error << response.body;
    return false;
  }
  return true;
}

//--------------------------------------------------------------------------
// Do an XML HTTP request on the given URL.
// If request is invalid (Element::none), no request body is sent
bool S3Client::do_request(const string& method,
                          const Web::URL& url,
                          const Misc::PropertyList& req_headers,
                          const XML::Element& req_xml,
                          XML::Element& resp_xml)
{
  string req_s = !req_xml?"":req_xml.to_string();
  string resp_s;

  if (!do_request(method, url, req_headers, req_s, resp_s)) return false;

  if (!resp_s.empty())
  {
    Log::Streams log;
    // Try to parse it
    XML::Parser parser(log.error);
    try
    {
      parser.read_from(resp_s);
    }
    catch (XML::ParseFailed)
    {
      log.error << "Bad XML in S3 response\n";
      return false;
    }

    resp_xml = parser.get_root();  // Copy
    OBTOOLS_LOG_IF_DEBUG(log.debug << resp_xml;)
  }

  return true;
}

//--------------------------------------------------------------------------
// Get S3 REST URL for a given bucket (or all buckets if empty) and object
// (or all objects if empty)
Web::URL S3Client::get_url(const string& bucket_name,
                           const string& object_key)
{
  if (bucket_name.empty())
    return Web::URL(string("http://")+s3_host+"/"+object_key);
  else if (use_virtual_hosts)
    return Web::URL(string("http://")+bucket_name+"."+s3_host+"/"+object_key);
  else
    return Web::URL(string("http://")+s3_host+"/"+bucket_name+"/"+object_key);
}

//--------------------------------------------------------------------------
// List all buckets owned by the user
bool S3Client::list_all_my_buckets(set<string>& buckets)
{
  XML::Element response;
  if (!do_request(get_url(), response)) return false;
  XML::XPathProcessor xpath(response);
  for(const auto bucket_e: xpath.get_elements("Buckets/Bucket"))
  {
    const auto& name_e = bucket_e->get_child("Name");
    if (!!name_e) buckets.insert(*name_e);
  }
  return true;
}

//--------------------------------------------------------------------------
// List a specific bucket
bool S3Client::list_bucket(const string& bucket_name,
                           set<string>& objects,
                           const string& prefix)
{
  Web::URL base_url = get_url(bucket_name);
  Misc::PropertyList query;
  if (!prefix.empty()) query.add("prefix", prefix);

  for(;;) // Loop on truncated response
  {
    Web::URL url = base_url;
    // Add query params if any
    url.append(query);

    // Add prefix if set
    XML::Element response;
    if (!do_request(url, response)) return false;
    XML::XPathProcessor xpath(response);
    for(const auto contents_e: xpath.get_elements("Contents"))
    {
      const auto& key_e = contents_e->get_child("Key");
      if (!!key_e)
      {
        objects.insert(*key_e);
        query.add("marker", *key_e);      // In case we're truncated
      }
    }

    // If not truncated, we're done
    if (!xpath.get_value_bool("IsTruncated")) return true;

    // Go round again with marker set
  }
}

//--------------------------------------------------------------------------
// Create a bucket
bool S3Client::create_bucket(const string& bucket_name,
                             const string& acl,
                             const string& region)
{
  Misc::PropertyList headers;
  if (!acl.empty()) headers.add("x-amz-acl", acl);

  if (region.empty())
  {
    // Simple case - no request
    string response;
    return do_request("PUT", get_url(bucket_name), headers, "", response);
  }
  else
  {
    XML::Element request("CreateBucketConfiguration");
    request.add("LocationConstraint", region);
    XML::Element response;
    return do_request("PUT", get_url(bucket_name), headers, request, response);
  }
}

//--------------------------------------------------------------------------
// Create an object
bool S3Client::create_object(const string& bucket_name,
                             const string& object_key,
                             const string& object_data,
                             const string& acl)
{
  string response;
  Misc::PropertyList headers;
  if (!acl.empty()) headers.add("x-amz-acl", acl);
  return do_request("PUT", get_url(bucket_name, object_key), headers,
                    object_data, response);
}

//--------------------------------------------------------------------------
// Get an object
bool S3Client::get_object(const string& bucket_name,
                          const string& object_key,
                          string& object_data)
{
  Misc::PropertyList headers;
  return do_request("GET", get_url(bucket_name, object_key), headers,
                    "", object_data);
}

//--------------------------------------------------------------------------
// Delete an object
bool S3Client::delete_object(const string& bucket_name,
                             const string& object_key)
{
  return do_request("DELETE", get_url(bucket_name, object_key));
}

//--------------------------------------------------------------------------
// Delete multiple objects
// Set max_keys_per_request for testing
bool S3Client::delete_multiple_objects(const string& bucket_name,
                                       set<string>& object_keys,
                                       int max_keys_per_request)
{
  Web::URL url = get_url(bucket_name);
  url.append("?delete=1");  // Note just ?delete doesn't sign properly

  // Shift keys into a vector so we can slice it
  vector<string> keys;
  for(const auto& key: object_keys)
    keys.push_back(key);

  int total = keys.size();
  for(int start=0; start<total; start+=max_keys_per_request)
  {
    XML::Element request("Delete");
    request.add("Quiet", "True");

    int n = min(total-start, max_keys_per_request);
    for(int i=start; i<start+n; i++)
      request.add("Object").add("Key", keys[i]);

    XML::Element response;
    Misc::PropertyList headers;
    if (!do_request("POST", url, headers, request, response))
      return false;

    // Check response for errors
    if (!response.get_children("Error").empty())
    {
      Log::Error log;
      log << "S3 multi-object delete failed:\n" << response;
      return false;
    }
  }

  return true;
}

//--------------------------------------------------------------------------
// Delete all objects with a given prefix
bool S3Client::delete_objects_with_prefix(const string& bucket_name,
                                          const string& prefix,
                                          int max_keys_per_request)
{
  set<string> keys;
  if (!list_bucket(bucket_name, keys, prefix)) return false;
  return delete_multiple_objects(bucket_name, keys, max_keys_per_request);
}

//--------------------------------------------------------------------------
// Empty the bucket
bool S3Client::empty_bucket(const string& bucket_name)
{
  set<string> keys;
  if (!list_bucket(bucket_name, keys)) return false;
  return delete_multiple_objects(bucket_name, keys);
}

//--------------------------------------------------------------------------
// Delete a bucket
bool S3Client::delete_bucket(const string& bucket_name)
{
  return do_request("DELETE", get_url(bucket_name));
}

}} // namespaces
