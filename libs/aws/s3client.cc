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
                                      true);  // empty body, but signed

  // Add query items
  request.url.get_query(req.query);

  // Sign it
  authenticator.sign(req);

  // Do the fetch
  return http.fetch(request, response);
}

}} // namespaces
