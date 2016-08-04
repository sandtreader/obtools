//==========================================================================
// ObTools::SOAP: url-handler.cc
//
// SOAP URL handler specialisations
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-soap.h"

namespace ObTools { namespace SOAP {

//--------------------------------------------------------------------------
// Implementation of general request handler
bool URLHandler::handle_request(const Web::HTTPMessage& http_request,
				Web::HTTPMessage& http_response,
				const SSL::ClientDetails& client)
{
  Log::Streams log;

  // Request must be POST
  if (http_request.method != "POST")
  {
    log.error << "SOAP server received bad request method: "
	      << http_request.method << endl;
    http_response.code = 405;
    http_response.reason = "Method not allowed";
    return true;
  }

  // Parse out SOAP body from request body
  Parser parser(log.error);

  // Add namespace fixes
  for(map<string, string>::iterator p = ns_map.begin(); p!=ns_map.end(); ++p)
    parser.fix_namespace(p->first, p->second);

  // Parse into message
  Message request(http_request.body, parser);

  // Make sure it's valid
  if (!request)
  {
    http_response.code = 400;
    http_response.reason = "Bad Request";
    return true;
  }

  // Call down to SOAP handler
  Message response(NS_ENVELOPE_1_1);  // Only support SOAP1.1 for now
  if (handle_message(request, response, http_request, http_response, client))
  {
    // Put response back into body
    http_response.body = response.to_string();
    return true;
  }
  else return false;
}

//--------------------------------------------------------------------------
// Handy support for generating faults - fills in response with fault
// Always returns true - use in return statements in handler
// e.g. return fault(SOAP::Fault::CODE_SENDER, "In your dreams, mate");
bool URLHandler::fault(Message& response, Fault::Code code,
		       const string& reason)
{
  Log::Streams log;
  Fault fault(code, reason);

  log.error << "SOAP Fault: " << fault.get_code_string()
	    << ": " << reason << endl;

  response.take(fault);
  return true;
}

}} // namespaces



