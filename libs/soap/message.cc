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
// Add a namespace attribute to the envelope
void Message::add_namespace(const string& attr, const string& value)
{
  if (doc) doc->set_attr(attr, value);
}

//------------------------------------------------------------------------
// Add a header element
// header is taken and will be deleted with message
// Returns reference to 'header'
XML::Element& Message::add_header(XML::Element *header)
{
  if (doc) doc->make_child("env:Header").add(header);
  return *header;
}

//------------------------------------------------------------------------
// Add a header element with given role string
// header is taken and will be deleted with message
// Element is modified with role, mustUnderstand and relay attributes
// Returns reference to created header element
XML::Element& Message::add_header(const string &name, const string& role,
				  bool must_understand,
				  bool relay)
{
  XML::Element *header = new XML::Element(name);

  // Only add role if non-empty
  if (!role.empty()) header->set_attr("env:role", role);

  // Only add mustUnderstand if true (SOAP 1.2: 5.2.3)
  if (must_understand) header->set_attr_bool("env:mustUnderstand", true);

  // Only add relay if true (SOAP 1.2: 5.2.4)
  if (relay) header->set_attr_bool("env:relay", true);

  return add_header(header);
}

//------------------------------------------------------------------------
// Add a header element with given standard role
// header is taken and will be deleted with message
// Element is modified with role, mustUnderstand and relay attributes
// Returns reference to created header element
XML::Element& Message::add_header(const string& name, Header::Role role,
				  bool must_understand,
				  bool relay)
{
  switch (role)
  {
    case Header::ROLE_NONE:
      return add_header(name, RN_NONE, must_understand, relay);

    case Header::ROLE_NEXT:
      return add_header(name, RN_NEXT, must_understand, relay);

    case Header::ROLE_ULTIMATE_RECEIVER:
      // UR is the default;  don't add it - SOAP 1.2: 5.2.2
      return add_header(name, "", must_understand, relay);
  }

  return XML::Element::none;
}

//------------------------------------------------------------------------
// Add a body element
// body is taken and will be deleted with message
XML::Element& Message::add_body(XML::Element *body)
{
  if (doc) doc->make_child("env:Body").add(body);
  return *body;
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
      h.relay           = he.get_attr_bool("env:relay");

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
