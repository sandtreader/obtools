//==========================================================================
// ObTools::SOAP: message-transport.cc
//
// Implementation of SOAP transport for message broker (see ot-msg.h)
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-soap.h"

namespace ObTools { namespace SOAP {

//==========================================================================
// TransportURLHandler class

//--------------------------------------------------------------------------
// Constructor - takes URL pattern
MessageTransportURLHandler::MessageTransportURLHandler(
          const string& _url, ObTools::Message::Handler& _handler): 
  URLHandler(_url), message_handler(_handler)
{
  fix_namespace(_handler.ns_prefix, _handler.ns_url);
}

//--------------------------------------------------------------------------
// Handle a SOAP message
bool MessageTransportURLHandler::handle_message(
                                         Message& request, Message& response,
					 Web::HTTPMessage&, Web::HTTPMessage&,
					 SSL::ClientDetails&)
{
  Log::Streams log;
  const XML::Element& body = request.get_body();

  // !!! Check body document name

  // Prepare response body, even if not used
  XML::Element *re;
  if (message_handler.complex_result)
    re = new XML::Element(message_handler.ns_prefix + ":"
			  + message_handler.document_name
			  + "-response",
			  "xmlns:" + message_handler.ns_prefix, 
			  message_handler.ns_url);
  else
    re = new XML::Element(message_handler.ns_prefix+":ok",
			  "xmlns:" + message_handler.ns_prefix, 
			  message_handler.ns_url);

  // Get handler to deal with message
  string error = message_handler.handle_message(body, *re);
  if (error.empty())
  {
    response.add_body(re);
    return true;
  }
  else return fault(response, SOAP::Fault::CODE_SENDER, error);
}

//==========================================================================
// Transport class

//--------------------------------------------------------------------------
// Register a handler with the given config element
void MessageTransport::register_handler(ObTools::Message::Handler& handler, 
					XML::Element& config)
{
  // Get URL
  string url = config["url"];

  // Create URL handler
  MessageTransportURLHandler *th = 
    new MessageTransportURLHandler(url, handler);
  server.add(th);
}

}} // namespaces
