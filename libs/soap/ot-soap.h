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
  Role role;
  bool must_understand;
  bool relay;
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
  // Add a namespace attribute to the envelope
  void add_namespace(const string& attr, const string& value);

  //------------------------------------------------------------------------
  // Add a header element (fully formed)
  // header is taken and will be deleted with message
  // Returns reference to 'header'
  XML::Element& add_header(XML::Element *header);

  //------------------------------------------------------------------------
  // Add a header element by name
  // Returns reference to created header element
  XML::Element& add_header(const string& name)
  { return add_header(new XML::Element(name)); }

  //------------------------------------------------------------------------
  // Add a header element with given role string
  // header is taken and will be deleted with message
  // Element is modified with role, mustUnderstand and relay attributes
  // Returns reference to created header element
  XML::Element& add_header(const string& name, const string& role,
			   bool must_understand = true,
			   bool relay = false);

  //------------------------------------------------------------------------
  // Add a header element with given standard role
  // header is taken and will be deleted with message
  // Element is modified with role, relay and mustUnderstand attributes
  // Returns reference to created header element
  XML::Element& add_header(const string& name, Header::Role role,
			   bool must_understand = true,
			   bool relay = false);

  //------------------------------------------------------------------------
  // Add a body element
  // body is taken and will be deleted with message
  // Returns reference to new body element
  XML::Element& add_body(XML::Element *body);

  //------------------------------------------------------------------------
  // Add a body element by name
  // Returns reference to created body element
  XML::Element& add_body(const string& name)
  { return add_body(new XML::Element(name)); }

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
// SOAP Fault Message
class Fault: public Message
{
public:
  //------------------------------------------------------------------------
  // Fault codes
  enum Code
  {
    CODE_VERSION_MISMATCH,
    CODE_MUST_UNDERSTAND,
    CODE_DATA_ENCODING_UNKNOWN,
    CODE_SENDER,
    CODE_RECEIVER
  };
  
  //------------------------------------------------------------------------
  // Constructor for outgoing faults 
  // Reason is the English (xml:lang="en") version - use add_reason for more
  Fault(Code code, const string& reason);

  //------------------------------------------------------------------------
  // Set a subcode 
  // According to SOAP 1.2: 5.4.1.3, the value should be a qualified name
  // We only allow one level here!
  void set_subcode(const string& value);

  //------------------------------------------------------------------------
  // Add a reason
  // Use this for non-English reasons - pass the English reason in the
  // constructor, above
  void add_reason(const string& text, const string& lang);

  //------------------------------------------------------------------------
  // Set the Node value
  // According to SOAP 1.2: 5.4.3, this should be a URI identifying the node
  // There should only be one (but this routine doesn't check this)
  void set_node(const string& uri);

  //------------------------------------------------------------------------
  // Set the Role value
  // According to SOAP 1.2: 5.4.4, this should be a URI identifying the role
  // the node was operating in when the fault occurred
  // There should only be one (but this routine doesn't check this)
  void set_role(const string& uri);

  //------------------------------------------------------------------------
  // Add a detail entry
  // Detail entries can be more or less anything
  void add_detail(XML::Element *detail);
};

//==========================================================================
// SOAP VersionMismatch Fault Message
// Adds recommended headers (SOAP1.2: 5.4.7), indicating support for
// ONLY SOAP1.2
class VersionMismatchFault: public Fault
{
public:
  //------------------------------------------------------------------------
  // Constructor for outgoing faults
  VersionMismatchFault();
};

//==========================================================================
// SOAP MustUnderstand Fault Message
// Adds recommended headers (SOAP1.2: 5.4.8), indicating non-understood
// elements
class MustUnderstandFault: public Fault
{
public:
  //------------------------------------------------------------------------
  // Constructor for outgoing faults
  MustUnderstandFault();

  //------------------------------------------------------------------------
  // Add a NotUnderstood block
  // attr/value indicate a namespace
  void add_not_understood(const string& qname, const string& attr,
			  const string& value);

};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_SOAP_H

















