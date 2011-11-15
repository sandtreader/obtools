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
  fix_namespace(_handler.ns_url, _handler.ns_prefix);
}

//--------------------------------------------------------------------------
// Handle a SOAP message
bool MessageTransportURLHandler::handle_message(
                                         Message& request, Message& response,
					 Web::HTTPMessage&, Web::HTTPMessage&,
					 SSL::ClientDetails& client)
{
  Log::Streams log;
  const XML::Element& body = request.get_body();

  // Check body document name, if specified
  if (!message_handler.document_name.empty() 
    && body.name != message_handler.ns_prefix + ":" 
                   + message_handler.document_name+"-request")
    return fault(response, SOAP::Fault::CODE_SENDER, "Bad document name");

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
  string error = message_handler.handle_message(body, client, *re);
  if (error.empty())
  {
    response.add_body(re);
    return true;
  }
  else
  {
    delete re;
    return fault(response, SOAP::Fault::CODE_SENDER, error);
  }
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

//==========================================================================
// Message interface

//--------------------------------------------------------------------------
// Constructor
MessageInterface::MessageInterface(XML::Element& config,
				   ObTools::Message::Broker& broker,
				   const string& server_name,
                                   SSL::Context *ssl_ctx): 
  http_server(0), http_server_thread(0),
  https_server(0), https_server_thread(0)
{
  XML::XPathProcessor xpath(config);
  Log::Streams log;

  // Start HTTP server
  int hport = xpath.get_value_int("server/@port", 0);
  if (hport)
  {
    // Default to localhost only
    Net::EndPoint addr(Net::IPAddress(xpath.get_value("server/@address",
						      "localhost")), hport);
    log.summary << "Starting HTTP SOAP server at " << addr << endl;
    http_server = new Web::SimpleHTTPServer(addr, server_name);

    // Add a message transport to message broker
    broker.add_transport(new MessageTransport(*http_server));

    // Start thread
    http_server_thread = new Net::TCPServerThread(*http_server);
  }

  // Start HTTPS server
  hport = xpath.get_value_int("ssl-server/@port", 0);
  if (hport)
  {
    if (ssl_ctx)
    {
      // Default to localhost only
      Net::EndPoint addr(Net::IPAddress(xpath.get_value("ssl-server/@address",
                                                        "localhost")), hport);
      log.summary << "Starting HTTPS SOAP server at " << addr << endl;
      https_server = new Web::SimpleHTTPServer(ssl_ctx, addr, server_name);

      // Add a message transport to message broker
      broker.add_transport(new MessageTransport(*https_server));

      // Start thread
      https_server_thread = new Net::TCPServerThread(*https_server);
    }
    else log.error << "SSL server requested but no SSL context established\n";
  }
}

//--------------------------------------------------------------------------
// Destructor - destroys SOAP interface
MessageInterface::~MessageInterface()
{
  if (http_server_thread) delete http_server_thread;
  if (http_server) delete http_server;
  if (https_server_thread) delete https_server_thread;
  if (https_server) delete https_server;
}

}} // namespaces
