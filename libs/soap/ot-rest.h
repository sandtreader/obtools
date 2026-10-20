//==========================================================================
// ObTools::REST: ot-rest.h
//
// Public definitions for ObTools::REST
// Basic REST-over-HTTP client
//
//==========================================================================

#ifndef __OBTOOLS_REST_H
#define __OBTOOLS_REST_H

#include <string>
#include "ot-web.h"

namespace ObTools { namespace REST {

using namespace std;

//==========================================================================
// HTTP Client class (http-client.cc)
class HTTPClient: public Web::HTTPClient
{
public:
  //------------------------------------------------------------------------
  // Constructor from server, no SSL
  HTTPClient(Net::EndPoint _server, const string& _ua="",
             int _connection_timeout=0, int _operation_timeout=0):
    Web::HTTPClient(_server, _ua, _connection_timeout, _operation_timeout) {}

  //------------------------------------------------------------------------
  // Constructor from server, with SSL
  HTTPClient(Net::EndPoint _server, SSL::Context *ctx, const string& _ua="",
             int _connection_timeout=0, int _operation_timeout=0):
    Web::HTTPClient(_server, ctx, _ua, _connection_timeout, _operation_timeout)
    {}

  //------------------------------------------------------------------------
  // Constructor from URL, no SSL
  HTTPClient(const Web::URL& url, const string& _ua="",
             int _connection_timeout=0, int _operation_timeout=0):
    Web::HTTPClient(url, 0, _ua, _connection_timeout, _operation_timeout) {}

  //------------------------------------------------------------------------
  // Constructor from URL, with SSL
  HTTPClient(const Web::URL& url, SSL::Context *ctx, const string& _ua="",
             int _connection_timeout=0, int _operation_timeout=0):
    Web::HTTPClient(url, ctx, _ua, _connection_timeout, _operation_timeout) {}

  //------------------------------------------------------------------------
  // POST an arbitrary request body with the given content type
  // Returns the HTTP status code, or a transport error code
  int post(const Web::URL& url, const string& request_body,
           string& response_body, const string& content_type);
};

}} // namespaces

#endif // !__OBTOOLS_REST_H