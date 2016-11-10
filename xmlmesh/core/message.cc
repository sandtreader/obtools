//==========================================================================
// ObTools::XMLMesh:Core: message.cc
//
// Implementation of XMLMesh message
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xmlmesh.h"
#include "ot-log.h"
#include "ot-misc.h"
#include <sstream>

namespace ObTools { namespace XMLMesh {

#define XMLMESH_ID_SIZE 16

//--------------------------------------------------------------------------
// Standard namespace
static const char *xmlmesh_namespace = "http://obtools.com/ns/xmlmesh";

//--------------------------------------------------------------------------
// Allocate an ID
// Actually, we just generate a very long random one - this should make it
// globally unique for correlation
static string _allocate_id()
{
  Misc::Random r;
  return r.generate_hex(XMLMESH_ID_SIZE);
}

//--------------------------------------------------------------------------
// Add routing header to the given SOAP message
static void _add_routing_header(SOAP::Message *soap,
                                const string& subject,
                                bool rsvp,
                                const string& ref)
{
  // Add routing header - role 'Next', must_understand, relay
  XML::Element& rh = soap->add_header("x:routing",
                                      SOAP::Header::ROLE_NEXT,
                                      true, true);

  // Add our routing parameters
  rh.set_attr("x:id", _allocate_id());
  rh.set_attr("x:subject", subject);
  if (rsvp) rh.set_attr_bool("x:rsvp", true);
  if (ref.size()) rh.set_attr("x:ref", ref);
}

//--------------------------------------------------------------------------
// Constructor from existing SOAP::Message for outgoing messages
// ID is manufactured here, and routing header added
// 'soap' is taken and will be disposed with message
Message::Message(const string& subject, SOAP::Message *soap,
                 bool rsvp, const string& ref)
{
  // Create a SOAP message with XMLMesh namespace
  soap_message = soap;
  soap->add_namespace("xmlns:x", xmlmesh_namespace);

  // Add routing header
  _add_routing_header(soap, subject, rsvp, ref);
}

//--------------------------------------------------------------------------
// Constructor from XML for outgoing messages
// ID is manufactured here
// Takes ownership of the XML::Element - make sure it's detached from the
// parser!
Message::Message(const string& subject, XML::Element *xml_content,
                 bool rsvp, const string& ref)
{
  // Create a SOAP message with XMLMesh namespace
  soap_message = new SOAP::Message();
  soap_message->add_namespace("xmlns:x", xmlmesh_namespace);

  // Add routing header
  _add_routing_header(soap_message, subject, rsvp, ref);

  // Add content body
  soap_message->add_body(xml_content);
}

//--------------------------------------------------------------------------
// Constructor from partial XML for outgoing messages
// ID is manufactured here
// Copies text of xml element and reparses it into body
Message::Message(const string& subject, const XML::Element& xml_content,
                 bool rsvp, const string& ref)
{
  // Create a SOAP message with XMLMesh namespace
  soap_message = new SOAP::Message();
  soap_message->add_namespace("xmlns:x", xmlmesh_namespace);

  // Add routing header
  _add_routing_header(soap_message, subject, rsvp, ref);

  // Manufacturer our own error stream since we're used multithreaded and
  // there's nothing long-lived here to provide a full Streams structure
  Log::Error error_log;

  // Parse content body
  XML::Parser parser(error_log);
  try
  {
    parser.read_from(xml_content.to_string());
    soap_message->add_body(parser.detach_root());
  }
  catch (const XML::ParseFailed&)
  {
    error_log << "XMLMesh Message creation: "
              << "can't reparse supplied element " << xml_content << endl;
  }
}

//--------------------------------------------------------------------------
// Constructor from partial XML text for outgoing messages
// ID is manufactured here
// body_text is the body text to be sent
Message::Message(const string& subject, const string& body_text,
                 bool rsvp, const string& ref)
{
  // Create a SOAP message with XMLMesh namespace
  soap_message = new SOAP::Message();
  soap_message->add_namespace("xmlns:x", xmlmesh_namespace);

  // Add routing header
  _add_routing_header(soap_message, subject, rsvp, ref);

  // Parse content body
  Log::Error error_log;
  XML::Parser parser(error_log);
  try
  {
    parser.read_from(body_text);
    XML::Element *root = parser.detach_root();
    if (root)
      soap_message->add_body(root);
    else
      error_log << "XMLMesh Message creation: No body XML found\n";
  }
  catch (const XML::ParseFailed&)
  {
    error_log << "XMLMesh Message creation: "
              << "can't parse supplied body text:\n"
              << body_text << endl;
  }
}

//--------------------------------------------------------------------------
// Constructor from XML <message> text for incoming messages
Message::Message(const string& message_text)
{
  textual_message = message_text;
  soap_message = 0;  // For now - will be created in get_xml if required
}

//--------------------------------------------------------------------------
// Get <message> text, but don't cache it
string Message::to_text() const
{
  if (textual_message.size()) return textual_message;

  // If XML exists, get the textual form
  if (soap_message) return soap_message->to_string();

  return "";
}

//--------------------------------------------------------------------------
// Get <message> text and cache it
string Message::get_text() const
{
  MT::Lock lock(cached_bits_mutex);
  if (!textual_message.size()) textual_message = to_text();

  return textual_message;
}

//--------------------------------------------------------------------------
// Get SOAP Message, still owned by Message, will be destroyed with it
// Check for validity with !
const SOAP::Message& Message::get_soap() const
{
  // If we've already got it, return immediately
  MT::Lock lock(cached_bits_mutex);
  if (soap_message) return *soap_message;

  // Create parser with fixed namespace in case someone's being clever
  // and using another prefix
  Log::Error error_log;
  SOAP::Parser parser(error_log);
  parser.fix_namespace(xmlmesh_namespace, "x");

  // Read SOAP message from parser
  soap_message = new SOAP::Message(textual_message, parser);
  if (!*soap_message)
  {
    error_log << "XMLMesh:: Can't parse incoming SOAP message\n";
    // Leave the dead XML here, otherwise we'll keep parsing it again
  }

  return *soap_message;
}

//--------------------------------------------------------------------------
// Get SOAP message for modification - clears textual copy if any
// SOAP is still owned by Message, will be destroyed with it
SOAP::Message& Message::get_modifiable_soap()
{
  // Make sure we have XML form available
  get_soap();

  // Delete the textual form
  textual_message = "";

  // Return message
  return *soap_message;
}

//--------------------------------------------------------------------------
// Get XML Body content, still owned by Message, will be destroyed with it
// Check for validity with !
XML::Element& Message::get_body() const
{
  get_soap();  // Ensure soap_message exists
  return soap_message->get_body();
}

//--------------------------------------------------------------------------
// Ditto, but specifying a particular element name
// Check for validity with !
XML::Element& Message::get_body(const string& name) const
{
  get_soap();  // Ensure soap_message exists
  return soap_message->get_body(name);
}

//--------------------------------------------------------------------------
// Get routing header of the message
const XML::Element& Message::get_routing_header() const
{
  const SOAP::Message& soap = get_soap();
  SOAP::Header h;
  if (soap.get_header("x:routing", h))
    return *h.content;
  else
    return XML::Element::none;
}

//--------------------------------------------------------------------------
// Get subject of a message
string Message::get_subject() const
{
  const XML::Element& routing = get_routing_header();
  return routing["x:subject"];
}

//--------------------------------------------------------------------------
// Get id of a message
string Message::get_id() const
{
  const XML::Element& routing = get_routing_header();
  return routing["x:id"];
}

//--------------------------------------------------------------------------
// Get whether the message requires a response
bool Message::get_rsvp() const
{
  const XML::Element& routing = get_routing_header();
  return routing.get_attr_bool("x:rsvp");
}

//--------------------------------------------------------------------------
// Get reference of a message
string Message::get_ref() const
{
  const XML::Element& routing = get_routing_header();
  return routing["x:ref"];
}

//--------------------------------------------------------------------------
// Destructor - kills xml data if not detached
Message::~Message()
{
  if (soap_message) delete soap_message;
}

//--------------------------------------------------------------------------
// << operator to write Message to ostream
ostream& operator<<(ostream& s, const Message& m)
{
  s << m.to_text();
  return s;
}

}} // namespaces





