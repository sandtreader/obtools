//==========================================================================
// ObTools::AWS: s3client.cc
//
// AWS S3 client implementation
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-aws.h"

namespace ObTools { namespace AWS {

//--------------------------------------------------------------------------
// Do an HTTP request, with authentication
bool S3Client::do_request(Web::HTTPMessage& request, Web::HTTPMessage &response)
{
  Web::HTTPClient http(request.url);

  // Add headers and authenticate
  request.headers.put("host", request.url.get_host());
  AWS::Authenticator::RequestInfo req(request.method,
                                      request.url.get_path(),
                                      Time::Stamp::now(),
                                      request.headers,
                                      true, request.body);

  // Add query items
  request.url.get_query(req.query);

  // Sign it
  authenticator.sign(req);

  // Do the fetch
  return http.fetch(request, response);
}

//--------------------------------------------------------------------------
// Do an XML HTTP request on the given URL.
// If request is invalid (Element::none), no request body is sent
bool S3Client::do_request(const string& method,
                          const Web::URL& url,
                          const XML::Element& req_xml,
                          XML::Element& resp_xml)
{
  Log::Streams log;
  Web::URL the_url = url;
  Web::HTTPMessage response;

  // Loop for redirects
  for(int redirect=0; redirect<2; redirect++)
  {
    log.detail << "S3 " << method << " " << the_url << endl;
    Web::HTTPMessage request(method, the_url);
    if (!!req_xml)
    {
      request.body = req_xml.to_string();
      OBTOOLS_LOG_IF_DEBUG(log.debug << request.body;)
    }
    if (!do_request(request, response)) return false;

    if (!response.body.empty())
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

      resp_xml = parser.get_root();  // Copy
      OBTOOLS_LOG_IF_DEBUG(log.debug << resp_xml;)
    }

    // Check for 307 Redirects when switching region, but only allow once
    if (!redirect && response.code == 307)
    {
      const string& endpoint = *resp_xml.get_child("Endpoint");
      if (endpoint.empty()) break;

      log.detail << "S3 307 redirect to " << endpoint << endl;

      // Just substitute into URL
      the_url = Web::URL(Text::subst(the_url.str(), the_url.get_host(),
                                     endpoint));
    }
    else break;
  }

  if (response.code/100 != 2) // Allow for 200, 204 etc.
  {
    log.error << "S3 request " << method << " " << url << " failed:\n";
    log.error << response.code << " " << response.reason << endl;
    log.error << resp_xml;
    return false;
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
  else
    return Web::URL(string("http://")+bucket_name+"."+s3_host+"/"+object_key);
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
                           set<string>& objects)
{
  XML::Element response;
  if (!do_request(get_url(bucket_name), response)) return false;
  XML::XPathProcessor xpath(response);
  for(const auto key_e: xpath.get_elements("Contents/Key"))
    objects.insert(**key_e);

  return true;
}

//--------------------------------------------------------------------------
// Create a bucket
bool S3Client::create_bucket(const string& bucket_name,
                             const string& region)
{
  if (region.empty())
  {
    // Simple case - no request
    return do_request("PUT", get_url(bucket_name));
  }
  else
  {
    XML::Element request("CreateBucketConfiguration");
    request.add("LocationConstraint", region);
    XML::Element response;
    return do_request("PUT", get_url(bucket_name), request, response);
  }
}

//--------------------------------------------------------------------------
// Delete a bucket
bool S3Client::delete_bucket(const string& bucket_name)
{
  return do_request("DELETE", get_url(bucket_name));
}

}} // namespaces
