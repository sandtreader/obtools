//==========================================================================
// ObTools::SOAP: message.cc
//
// Implementation of SOAP message
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-soap.h"

namespace ObTools { namespace SOAP {

//------------------------------------------------------------------------
// Default constructor - empty header and body
Message::Message()
{
  doc = new XML::Element("env:Envelope");
  doc->set_attr("xmlns:env", NS_ENVELOPE);
  doc->add("env:Header");
  doc->add("env:Body");
}

//------------------------------------------------------------------------
// Constructor from XML text, using the given output stream for errors
Message::Message(const string& text, ostream& err_s)
{
  Parser p(err_s);

  try
  {
    p.read_from(text);
  }
  catch (ObTools::XML::ParseFailed)
  {
    err_s << "XML parse failed" << endl;
    doc = 0;
    return;
  }

  doc = p.detach_root();
}

//------------------------------------------------------------------------
// Constructor from input stream, using the given output stream for errors
Message::Message(istream& in_s, ostream& err_s)
{
  Parser p(err_s);

  try
  {
    in_s >> p;
  }
  catch (ObTools::XML::ParseFailed)
  {
    err_s << "XML parse failed" << endl;
    doc = 0;
    return;
  }

  doc = p.detach_root();
}

//------------------------------------------------------------------------
// Add a header element
// header is taken and will be deleted with message
void Message::add_header(XML::Element *header)
{
  if (doc)
  {
    // Make sure there is an env:Header container
    if (!doc->get_child("env:Header")) doc->add("env:Header");
    XML::Element& eh = doc->get_child("env:Header");
    if (!!eh) eh.add(header);
  }
  else
  {
    // Safest to clean up here
    delete header;
  }
}

//------------------------------------------------------------------------
// Add a header element with given role string
// header is taken and will be deleted with message
// Element is modified with role and mustUnderstand attributes
void Message::add_header(XML::Element *header, const string& role,
			 bool must_understand)
{
  header->set_attr("env:role", role);

  // Only add mustUnderstand if true (SOAP 1.2: 5.2.3)
  if (must_understand) header->set_attr_bool("env:mustUnderstand", true);
  add_header(header);
}

//------------------------------------------------------------------------
// Add a header element with given standard role
// header is taken and will be deleted with message
// Element is modified with role and mustUnderstand attributes
void Message::add_header(XML::Element *header, Header::Role role,
			 bool must_understand)
{
  switch (role)
  {
    case Header::ROLE_NONE:
      add_header(header, RN_NONE, must_understand);
      break;

    case Header::ROLE_NEXT:
      add_header(header, RN_NEXT, must_understand);
      break;

    case Header::ROLE_ULTIMATE_RECEIVER:
      // UR is the default;  don't add it - SOAP 1.2: 5.2.2
      if (must_understand) header->set_attr_bool("env:mustUnderstand", true);
      add_header(header);
      break;
  }
}

//------------------------------------------------------------------------
// Add a body element
// body is taken and will be deleted with message
void Message::add_body(XML::Element *body)
{
  if (doc)
  {
    // Make sure there is an env:Body container
    if (!doc->get_child("env:Body")) doc->add("env:Body");
    XML::Element& eb = doc->get_child("env:Body");
    if (!!eb) eb.add(body);
  }
  else
  {
    // Safest to clean up here
    delete body;
  }
}

//------------------------------------------------------------------------
// Dump to given output stream
void Message::write_to(ostream& s) const 
{ 
  if (doc)
    doc->write_to(s); 
  else
    s << "INVALID SOAP!\n";
} 

//------------------------------------------------------------------------
// Output to XML text
string Message::to_string() const 
{ 
  if (doc)
    return doc->to_string(); 
  else
    return "INVALID SOAP!\n";
}

//------------------------------------------------------------------------
// << operator to write Message to ostream
ostream& operator<<(ostream& s, const Message& m)
{
  m.write_to(s);
  return s;
}

//------------------------------------------------------------------------
// Get first (or only) body element
// Returns Element::none if none
XML::Element& Message::get_body()
{
  if (doc)
  {
    XML::Element& body = doc->get_child("env:Body");
    if (!!body) return body.get_child();  // Whatever it is
  }

  return XML::Element::none;
}

//------------------------------------------------------------------------
// Get list of body elements
list<XML::Element *> Message::get_bodies()
{
  if (doc)
  {
    XML::Element& body = doc->get_child("env:Body");
    return body.children;  // Whatever they are (empty if body is invalid)
  }

  list<XML::Element *> empty;
  return empty;
}

//------------------------------------------------------------------------
// Get list of header elements
list<Header> Message::get_headers()
{
  list<Header> headers;

  if (doc)
  {
    XML::Element& header = doc->get_child("env:Header");

    // Inspect all header blocks for standard attributes
    OBTOOLS_XML_FOREACH_CHILD(he, header)
      Header h;
      h.content = &he;
      h.must_understand = he.get_attr_bool("env:mustUnderstand");
      string rs = he["env:role"];

      if (rs == RN_NONE)
	h.role = Header::ROLE_NONE;
      else if (rs == RN_NEXT)
	h.role = Header::ROLE_NEXT;
      // UR is default - SOAP 1.2: 5.2.2
      else if (rs.empty() || rs == RN_ULTIMATE_RECEIVER)  
	h.role = Header::ROLE_ULTIMATE_RECEIVER;
      else
	h.role = Header::ROLE_OTHER;

      headers.push_back(h);
    OBTOOLS_XML_ENDFOR
  }

  return headers;
}

}} // namespaces
