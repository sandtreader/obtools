//==========================================================================
// ObTools::SOAP: http-client.cc
//
// SOAP HTTP client specialisation
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-soap.h"
#include "ot-log.h"

namespace ObTools { namespace SOAP {

//--------------------------------------------------------------------------
// Simple request POST operation on a specified URL and optional SOAPAction
// Returns result code, fills in response
int HTTPClient::post(Web::URL& url, const string& soap_action,
		     Message& request, Message& response)
{
  Log::Streams log;

  // Create HTTP request from SOAP message
  Web::HTTPMessage http_request("POST", url);
  http_request.body = request.to_string();

  // Add SOAPAction header and XML content type
  if (!soap_action.empty())
    http_request.headers.put("SOAPAction", soap_action);

  http_request.headers.put("Content-Type", "text/xml; charset=utf-8");

  // Do normal POST operation
  Web::HTTPMessage http_response;
  if (!fetch(http_request, http_response)) return 400;

  // Parse out SOAP result from response_body
  Parser parser(log.error);

  // Add namespace mappings
  for(map<string, string>::const_iterator p = ns_map.begin();
      p!=ns_map.end(); ++p)
    parser.fix_namespace(p->first, p->second);

  Message soap_response(http_response.body, parser);

  // Make sure it's valid
  if (!soap_response)
  {
    log.error << "Invalid SOAP returned from " << server << ":\n"
	      << http_response.body << endl;
    return 500;
  }

  // Take it to return
  response.take(soap_response);
  return http_response.code;
}


}} // namespaces



