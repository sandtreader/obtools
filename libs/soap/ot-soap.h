//==========================================================================
// ObTools::SOAP: ot-soap.h
//
// Public definitions for ObTools::SOAP
// Support for SOAP messages
// 
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_SOAP_H
#define __OBTOOLS_SOAP_H

#include <string>
#include <list>
#include <map>
#include <ot-xml.h>

namespace ObTools { namespace SOAP { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Specification constants

// Namespaces
const char NS_ENVELOPE[] = "http://www.w3.org/2003/05/soap-envelope";

// Role names
const char RN_NONE[] = "http://www.w3.org/2003/05/soap-envelope/role/none";
const char RN_NEXT[] = "http://www.w3.org/2003/05/soap-envelope/role/next";
const char RN_ULTIMATE_RECEIVER[] = 
  "http://www.w3.org/2003/05/soap-envelope/role/ultimateReceiver";

//==========================================================================
// SOAP XML parser
// Standard XML parser with namespace fixes
class Parser: public XML::Parser
{
public:

  //------------------------------------------------------------------------
  // Constructor - use given stream for errors, set namespace
  Parser(ostream& s): XML::Parser(s)
  { fix_namespace(NS_ENVELOPE, "env"); }
};

//==========================================================================
// SOAP Header information
struct Header
{
  enum Role
  {
    ROLE_NONE,
    ROLE_NEXT,
    ROLE_ULTIMATE_RECEIVER,
    ROLE_OTHER                 // Application defined
  };

  XML::Element *content;  // The XML element
  bool must_understand;
  Role role;
};

//==========================================================================
// SOAP Message
class Message
{
private:
  XML::Element *doc;

public:
  //------------------------------------------------------------------------
  // Default constructor - empty header and body
  Message();

  //------------------------------------------------------------------------
  // Constructor from XML text, using the given output stream for errors
  Message(const string& text, ostream& err_s);

  //------------------------------------------------------------------------
  // Constructor from input stream, using the given output stream for errors
  Message(istream& in_s, ostream& err_s);

  //------------------------------------------------------------------------
  // Add a header element (fully formed)
  // header is taken and will be deleted with message
  void add_header(XML::Element *header);

  //------------------------------------------------------------------------
  // Add a header element with given role string
  // header is taken and will be deleted with message
  // Element is modified with role and mustUnderstand attributes
  void add_header(XML::Element *header, const string& role,
		  bool must_understand = true);

  //------------------------------------------------------------------------
  // Add a header element with given standard role
  // header is taken and will be deleted with message
  // Element is modified with role and mustUnderstand attributes
  void add_header(XML::Element *header, Header::Role role,
		  bool must_understand = true);

  //------------------------------------------------------------------------
  // Add a body element
  // body is taken and will be deleted with message
  void add_body(XML::Element *body);

  //------------------------------------------------------------------------
  // Dump XML text to given output stream
  void write_to(ostream& s) const;

  //------------------------------------------------------------------------
  // Output to XML text
  string to_string() const;

  //------------------------------------------------------------------------
  // Get first (or only) body element
  // Returns Element::none if none
  XML::Element& get_body();

  //------------------------------------------------------------------------
  // Get list of body elements
  list<XML::Element *> get_bodies();

  //------------------------------------------------------------------------
  // Get list of headers, parsed out into Header structures
  list<Header> get_headers();

  //------------------------------------------------------------------------
  // Destructor
  ~Message() { if (doc) delete doc; }

};

//------------------------------------------------------------------------
// << operator to write Message to ostream
ostream& operator<<(ostream& s, const Message& m);

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_SOAP_H
















