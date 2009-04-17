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
Message::Message(const string& ns)
{
  doc = new XML::Element("env:Envelope");
  doc->set_attr("xmlns:env", ns);
  doc->add("env:Header");
  doc->add("env:Body");
}

//------------------------------------------------------------------------
// Constructor from XML text, using the given parser
Message::Message(const string& text, Parser& p): doc(0)
{
  try
  {
    p.read_from(text);
    if (p.verify()) doc = p.detach_root();
  }
  catch (ObTools::XML::ParseFailed)
  {}
}

//------------------------------------------------------------------------
// Constructor from input stream, using the given parser
Message::Message(istream& in_s, Parser& p): doc(0)
{
  try
  {
    in_s >> p;
    if (p.verify()) doc = p.detach_root();
  }
  catch (ObTools::XML::ParseFailed)
  {}
}

//------------------------------------------------------------------------
// Replace with another message - like a copy constructor, but explicit
// and destroys the original
void Message::take(Message& original) 
{ 
  if (doc) delete doc;
  doc=original.doc; 
  original.doc=0; 
}

//------------------------------------------------------------------------
// Add a namespace attribute to the envelope
void Message::add_namespace(const string& attr, const string& value)
{
  if (doc) doc->set_attr(attr, value);
}

//------------------------------------------------------------------------
// Add standard namespaces for WSDL-style SOAP
void Message::add_wsdl_namespaces()
{
  add_namespace("xmlns:soapenc", "http://schemas.xmlsoap.org/soap/encoding/");
  add_namespace("xmlns:wsdl", "http://schemas.xmlsoap.org/wsdl/");
  add_namespace("xmlns:wsdlsoap", "http://schemas.xmlsoap.org/wsdl/soap/");
  add_namespace("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
  add_namespace("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
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

    default:
      return XML::Element::none;
  }
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
// Add a WSDL-style body element with a given name and namespace, plus
// a standard SOAP encodingStyle attribute
// body is taken and will be deleted with message
XML::Element& Message::add_wsdl_body(const string& name,
				     const string& ns_prefix,
				     const string& ns)
{
  XML::Element *body = new XML::Element(name, "xmlns:"+ns_prefix, ns);
  if (doc)
  {
    XML::Element& env_body = doc->make_child("env:Body");
    env_body.add(body);
    env_body.set_attr("env:encodingStyle", 
		      "http://schemas.xmlsoap.org/soap/encoding/");
  }

  return *body;
}

//------------------------------------------------------------------------
// Dump to given output stream
void Message::write_to(ostream& s) const 
{ 
  if (doc)
    doc->write_to(s, true); 
  else
    s << "INVALID SOAP!\n";
} 

//------------------------------------------------------------------------
// Output to XML text
string Message::to_string() const 
{ 
  if (doc)
    return doc->to_string(true); 
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
// Flatten any href/id (SOAP1.1) reference structure, taking copies of 
// referenced elements and replacing referencing elements with them, thus
// creating the inline equivalent document.
// Leaves any references to ancestors (loops) alone
// Modifies all bodies in place
void Message::flatten_bodies()
{
  if (!doc) return;

  for(XML::Element::iterator p(doc->get_children("env:Body")); p; ++p)
  {
    XML::Element& body = *p;

    // Recursively search for id attributes
    map<string, XML::Element *> ids;
    fill_id_map(body, ids);

    // Now fix up all elements with href attributes
    fix_hrefs(body, ids);
  }
}

//------------------------------------------------------------------------
// Recurse a (sub)-document looking for id attributes and filling in the
// given map
void Message::fill_id_map(XML::Element& e, map<string, XML::Element *>& ids)
{
  // Does this one have an ID?
  if (e.has_attr("id")) ids[e["id"]] = &e;

  // Recurse to children
  for(XML::Element::iterator p(e.children); p; ++p)
    fill_id_map(*p, ids);
}

//------------------------------------------------------------------------
// Fix up a (sub)-document looking for href attributes and replacing the
// element containing it with a copy of the element referred to
void Message::fix_hrefs(XML::Element& e, map<string, XML::Element *>& ids)
{
  // Does this one have an href?
  if (e.has_attr("href"))
  {
    string frag = e["href"];

    // Lose # at front
    if (!frag.empty() && frag[0] == '#') frag = string(frag, 1);

    // Get referred element
    XML::Element *ref = ids[frag];
    if (!ref) return;

    // Make sure this isn't 'e' or an ancestor of 'e', to prevent loops
    for(XML::Element *pe = &e; pe; pe=pe->parent)
      if (pe == ref) return;

    // Replace this element with a copy of the referred one, but with
    // the name changed to that of the referrer
    XML::Element *copy = ref->deep_copy();
    copy->name = e.name;

    // Recurse to this to replace hrefs inside it
    fix_hrefs(*copy, ids);

    // Replace element with this 
    e.replace_with(copy);

    // Delete old element (e goes invalid here)
    delete &e;
  }
  else 
  {
    // Recurse to children
    for(XML::Element::iterator p(e.children); p; ++p)
      fix_hrefs(*p, ids);
  }
}

//------------------------------------------------------------------------
// Get first (or only) body element
// Returns Element::none if none
XML::Element& Message::get_body() const
{
  if (doc)
  {
    XML::Element& body = doc->get_child("env:Body");
    if (!!body) return body.get_child();  // Whatever it is
  }

  return XML::Element::none;
}

//------------------------------------------------------------------------
// Get first (or only) body element of the given name
// Returns Element::none if none
XML::Element& Message::get_body(const string& name) const
{
  if (doc)
  {
    XML::Element& body = doc->get_child("env:Body");
    if (!!body) return body.get_child(name);
  }

  return XML::Element::none;
}

//------------------------------------------------------------------------
// Get list of body elements
list<XML::Element *> Message::get_bodies() const
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
// Convert a header element into a Header structure
static Header _read_header(const XML::Element& he)
{
  bool must_understand = he.get_attr_bool("env:mustUnderstand");
  bool relay           = he.get_attr_bool("env:relay");

  string rs = he["env:role"];
  Header::Role role;

  if (rs == RN_NONE)
    role = Header::ROLE_NONE;
  else if (rs == RN_NEXT)
    role = Header::ROLE_NEXT;
  // UR is default - SOAP 1.2: 5.2.2
  else if (rs.empty() || rs == RN_ULTIMATE_RECEIVER)  
    role = Header::ROLE_ULTIMATE_RECEIVER;
  else
    role = Header::ROLE_OTHER;

  return Header(&he, role, must_understand, relay);
}

//------------------------------------------------------------------------
// Get list of header elements
list<Header> Message::get_headers() const
{ 
  list<Header> headers;

  if (doc)
  {
    XML::Element& header = doc->get_child("env:Header");

    // Inspect all header blocks for standard attributes
    OBTOOLS_XML_FOREACH_CHILD(he, header)
      headers.push_back(_read_header(he));
    OBTOOLS_XML_ENDFOR
  }

  return headers;
}

//------------------------------------------------------------------------
// Get a single header of a particular name
// Returns whether successful;  fills in h if so
bool Message::get_header(const string& name, Header& h) const
{
  if (doc)
  {
    XML::Element& header = doc->get_child("env:Header");
    XML::Element& he = header.get_child(name);
    if (!!he)
    {
      h = _read_header(he);
      return true;
    }
  }

  return false;
}


}} // namespaces
