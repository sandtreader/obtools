//==========================================================================
// ObTools::SOAP: ot-soap.h
//
// Public definitions for ObTools::SOAP
// Support for SOAP messages
// 
// Copyright (c) 2003-2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_SOAP_H
#define __OBTOOLS_SOAP_H

#include <string>
#include <list>
#include <map>
#include "ot-xml.h"
#include "ot-web.h"

namespace ObTools { namespace SOAP { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Specification constants

// Namespaces
const char NS_ENVELOPE_1_1[] = "http://schemas.xmlsoap.org/soap/envelope/";
const char NS_ENVELOPE_1_2[] = "http://www.w3.org/2003/05/soap-envelope";

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
  // Constructor - use given stream for errors
  Parser(ostream& s);

  //------------------------------------------------------------------------
  // Verify the document is valid soap
  bool verify();
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

  const XML::Element *content;  // The XML element
  Role role;
  bool must_understand;
  bool relay;

  //------------------------------------------------------------------------
  // Constructors
  Header(): content(0), role(ROLE_NONE), must_understand(false), relay(false) 
  {}

  Header(const XML::Element *_content, Role _role, bool _mu, bool _relay):
    content(_content), role(_role), must_understand(_mu), relay(_relay) 
  {}
};

//==========================================================================
// SOAP Message
class Message
{
private:
  XML::Element *doc;

  // Internals
  void fill_id_map(XML::Element& e, map<string, XML::Element *>& ids);
  void fix_hrefs(XML::Element& e, map<string, XML::Element *>& ids);

public:
  //------------------------------------------------------------------------
  // Default constructor - empty header and body
  // Provide envelope namespace if not 1_2 (www.w3.org...)
  Message(const string& ns = NS_ENVELOPE_1_2);

  //------------------------------------------------------------------------
  // Constructor from XML text, using the given parser
  Message(const string& text, Parser& p);

  //------------------------------------------------------------------------
  // Constructor from input stream, using the given parser
  Message(istream& in_s, Parser& p);

  //------------------------------------------------------------------------
  // Check for validity
  bool valid() { return doc!=0; }
  bool operator!() { return !valid(); }

  //------------------------------------------------------------------------
  // Replace with another message - like a copy constructor, but explicit
  // and destroys the original
  void take(Message& original);

  //------------------------------------------------------------------------
  // Add a namespace attribute to the envelope
  void add_namespace(const string& attr, const string& value);

  //------------------------------------------------------------------------
  // Add standard namespaces for WSDL-style SOAP
  void add_wsdl_namespaces();

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
  // Flatten any href/id (SOAP1.1) reference structure, taking copies of 
  // referenced elements and replacing referencing elements with them, thus
  // creating the inline equivalent document.
  // Leaves any references to ancestors (loops) alone
  // Modifies all bodies in place
  void flatten_bodies();

  //------------------------------------------------------------------------
  // Get first (or only) body element
  // Returns Element::none if none
  XML::Element& get_body() const;

  //------------------------------------------------------------------------
  // Get first (or only) body element of the given name
  // Returns Element::none if none
  XML::Element& get_body(const string& name) const;

  //------------------------------------------------------------------------
  // Add a WSDL-style body element with a given name and namespace, plus
  // a standard SOAP encodingStyle attribute
  // body is taken and will be deleted with message
  XML::Element& add_wsdl_body(const string& name,
			      const string& ns_prefix,
			      const string& ns);

  //------------------------------------------------------------------------
  // Get list of body elements
  list<XML::Element *> get_bodies() const;

  //------------------------------------------------------------------------
  // Get list of headers, parsed out into Header structures
  list<Header> get_headers() const;

  //------------------------------------------------------------------------
  // Get a single header of a particular name
  // Returns whether successful;  fills in h if so
  bool get_header(const string& name, Header& h) const;

  //------------------------------------------------------------------------
  // Destructor
  virtual ~Message() { if (doc) delete doc; }
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
    CODE_RECEIVER,
    CODE_UNKNOWN
  };
  
  //------------------------------------------------------------------------
  // Constructor for outgoing faults 
  // Reason is the English (xml:lang="en") version - use add_reason for more
  Fault(Code code, const string& reason);

  //------------------------------------------------------------------------
  // Constructor from XML text, using the given parser
  Fault(const string& text, Parser& p): Message(text, p) {}

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

  //------------------------------------------------------------------------
  // Get code string from incoming fault
  // Returns empty string if no code found
  string get_code_string();

  //------------------------------------------------------------------------
  // Get code from incoming fault
  Code get_code();

  //------------------------------------------------------------------------
  // Get reason from incoming fault with language code given
  string get_reason(const string& lang="en");
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
// HTTP Client class (http-client.cc)
// Extends Web::HTTPClient to provide SOAP interface
class HTTPClient: public Web::HTTPClient
{
public:
  //--------------------------------------------------------------------------
  // Constructor from server, no SSL
  HTTPClient(Net::EndPoint _server, const string& _ua="", 
	     int _connection_timeout=0, int _operation_timeout=0): 
    Web::HTTPClient(_server, _ua, _connection_timeout, _operation_timeout) {}

  //--------------------------------------------------------------------------
  // Constructor from server, with SSL
  HTTPClient(Net::EndPoint _server, SSL::Context *ctx, const string& _ua="",
	     int _connection_timeout=0, int _operation_timeout=0): 
    Web::HTTPClient(_server, ctx, _ua, _connection_timeout, _operation_timeout)
    {}

  //--------------------------------------------------------------------------
  // Constructor from URL, no SSL - extracts server from host/port parts
  HTTPClient(Web::URL& url, const string& _ua="",
	     int _connection_timeout=0, int _operation_timeout=0): 
    Web::HTTPClient(url, 0, _ua, _connection_timeout, _operation_timeout) {}

  //--------------------------------------------------------------------------
  // Constructor from URL, with SSL - extracts server from host/port parts
  HTTPClient(Web::URL& url, SSL::Context *ctx, const string& _ua="",
	     int _connection_timeout=0, int _operation_timeout=0): 
    Web::HTTPClient(url, ctx, _ua, _connection_timeout, _operation_timeout) {}

  //--------------------------------------------------------------------------
  // Simple request POST operation on a specified URL and SOAP action
  // Returns result code, fills in response
  int post(Web::URL& url, const string& soap_action,
	   Message& request, Message& response);

  //--------------------------------------------------------------------------
  // Simple request POST operation to root URL
  // Returns result code, fills in response
  int post(const string& soap_action, Message& request, Message& response)
  { Web::URL url("/"); return post(url, soap_action, request, response); }

};

//==========================================================================
// SOAP URL handler abstract class (url-handler.cc)
// Like Web::URLHandler, but provides SOAP message interface
// Use with standard Web::SimpleHTTPServer, just like Web::URLHandler
class URLHandler: public Web::URLHandler
{
private:
  // Namespace map for Parser
  map<string, string> ns_map;

  //--------------------------------------------------------------------------
  // Implementation of standard HTTP handler
  bool handle_request(Web::HTTPMessage& http_request, 
		      Web::HTTPMessage& http_response,
		      SSL::ClientDetails& client);

protected:
  //--------------------------------------------------------------------------
  // Abstract interface to handle SOAP messages
  // http_request, http_response and client are made available for complex 
  // use, but can be ignored
  virtual bool handle_message(Message& request, Message& response,
			      Web::HTTPMessage& http_request,
			      Web::HTTPMessage& http_response,
			      SSL::ClientDetails& client) = 0;

  //--------------------------------------------------------------------------
  // Handy support for generating faults - fills in response with fault
  // Always returns true - use in return statements in handler
  // e.g. return fault(SOAP::Fault::CODE_SENDER, "In your dreams, mate");
  bool fault(Message& response, Fault::Code code, const string& reason);

public:
  //--------------------------------------------------------------------------
  // Constructor - takes URL pattern
  URLHandler(const string& _url): Web::URLHandler(_url) {}

  //--------------------------------------------------------------------------
  // Register namespace translation (see ot-xml.h)
  void fix_namespace(const string& name, const string& prefix)
  { ns_map[name] = prefix; }

  //--------------------------------------------------------------------------
  // Virtual destructor
  virtual ~URLHandler() {}
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_SOAP_H

















