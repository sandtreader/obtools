//==========================================================================
// ObTools::XMLMesh::Client: message-transport.cc
//
// Implementation of Mesh transport for message broker (see ot-msg.h)
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xmlmesh-client.h"

namespace ObTools { namespace XMLMesh {

//==========================================================================
// TransportURLHandler class

//--------------------------------------------------------------------------
// Constructor - takes subject
MessageTransportSubscriber::MessageTransportSubscriber(
			   const string& _subject, 
			   MultiClient& client,
			   ObTools::Message::Handler& _handler):
  Subscriber(client, _subject+".request"), 
  message_handler(_handler)
{

}

//--------------------------------------------------------------------------
// Handle a message
void MessageTransportSubscriber::handle(Message& msg)
{
  Log::Streams log;
  const XML::Element& request = msg.get_body();

  // Check body document name, if specified
  if (!message_handler.document_name.empty()
   && request.name != message_handler.ns_prefix + ":" 
                    + message_handler.document_name+"-request")
  {
    client.respond(SOAP::Fault::CODE_SENDER, "Bad document name", msg);
    return;
  }

  // Prepare response body, even if not used
  XML::Element response(message_handler.ns_prefix+":"
			+message_handler.document_name+"-response",
			"xmlns:" + message_handler.ns_prefix, 
			message_handler.ns_url);

  // Get handler to deal with message
  string error = message_handler.handle_message(request, response);
  if (error.empty())
  {
    if (message_handler.complex_result)
    {
      // Change .request to .response
      string orig_subject(subject, 0, subject.size()-8);

      // Send correlated response
      XMLMesh::Message resp(orig_subject+".response", response, 
			    false, msg.get_id());
      client.send(resp); 
    }
    else
    {
      // Simple OK
      client.respond(msg);
    }
  }
  else 
    client.respond(SOAP::Fault::CODE_SENDER, error, msg);
}

//==========================================================================
// Transport class

//--------------------------------------------------------------------------
// Register a handler with the given config element
void MessageTransport::register_handler(ObTools::Message::Handler& handler, 
					XML::Element& config)
{
  // Get subject
  string subject = config["subject"];

  // Create URL handler
  MessageTransportSubscriber *sub = 
    new MessageTransportSubscriber(subject, client, handler);
  subscribers.push_back(sub);
}

//--------------------------------------------------------------------------
// Destructor
MessageTransport::~MessageTransport()
{
  // Destroy subscribers
  for(list<XMLMesh::Subscriber *>::iterator p = subscribers.begin();
      p != subscribers.end(); ++p)
    (*p)->disconnect();
}


}} // namespaces
