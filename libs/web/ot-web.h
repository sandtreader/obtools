//==========================================================================
// ObTools::Net: ot-web.h
//
// Public definitions for ObTools::Web
// Web protocols parsers and helpers
// 
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_WEB_H
#define __OBTOOLS_WEB_H

#include <stdint.h>
#include <iostream>
#include <string>
#include "ot-xml.h"

#if !defined(_SINGLE)
#include "ot-mt.h"
#endif

namespace ObTools { namespace Web { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// MIME header parser (mime-headers.cc)
// Reads a block of MIME headers - e.g. from a mail message, or an HTTP
// request.  Outputs in XML, since we have convenient access & iteration
// methods for this.
// Create the header parser around the root XML element:
// e.g.
//   XML::Element root("MIME");
//   Web::MimeHeaderParser mhp(root, cin);
//   mhp.parse();
//   ... use sub-elements of root, e.g. through XPath ...

// Each header is added as a sub-element of the root, with name (tag)
// equal to the header tag **lowercased**, and content equal to the text 
// of the header

class MIMEHeaderParser
{
private:
  static const unsigned int MAX_HEADER = 8000;  // DoS protection
  XML::Element& root;
  istream& in;

public:
  //--------------------------------------------------------------------------
  // Constructor/Destructor
  MIMEHeaderParser(XML::Element& _root, istream& _in): root(_root), in(_in) {}
  ~MIMEHeaderParser() {}

  //--------------------------------------------------------------------------
  // Get a line
  // Returns true if read OK - even if blank
  // Exported for the convenience of HTTP reader - see below
  bool getline(string& s);

  //--------------------------------------------------------------------------
  // Parse the stream
  // Returns whether successful
  // Skips the blank line delimiter, leaving stream ready to read message
  // body (if any)
  bool parse();
};

//==========================================================================
// MIME header generator (mime-headers.cc)
// Writes a block of MIME headers - e.g. for a mail message, or an HTTP
// request.  Gets the structure from XML like that returned from a
// MIMEHeaderParser
//
// Create the header generator around the root XML element:
// e.g.
//   XML::Element root("MIME");
//   ... fill up with root.add(name, value); ...
//
//   Web::MimeHeaderGenerator mhg(root, cout);
//   mhp.generate();

// Header names are generated with first letter capitalised, following 
// convention.  Values longer than 64 characters are folded at commas 
// (if any) or spaces (if any)             

class MIMEHeaderGenerator
{
private:
  static const unsigned int MAX_LINE = 64;
  XML::Element& root;
  ostream& out;

public:
  //--------------------------------------------------------------------------
  // Constructor/Destructor
  MIMEHeaderGenerator(XML::Element& _root, ostream& _out): 
    root(_root), out(_out) {}
  ~MIMEHeaderGenerator() {}

  //--------------------------------------------------------------------------
  // Generates the stream
  // Returns whether successful (can only fail if stream fails)
  // Includes the blank line delimiter, leaving stream ready to write message
  // body (if any)
  bool generate();
};

//==========================================================================
// URL parser (url.cc)
// Parses a URL into an XML structure

// Create the URL parser around a root XML element:
// e.g.
//   XML::Element root("url");
//   Web::URLParser urlp(root);
//   string url = "http://foo.com:81/x/y/z?bar#1";
//   urlp.parse(url);
//   ... use sub-elements of root, e.g. through XPath ...

// the root element is given subelements as follows
//   scheme    Scheme (e.g. http) - lower cased
//   host      Host/domain (if present) - lower cased
//   port      Port number (if present)
//   path      URL path (including '/' if absolute) (as specified)
//   fragment  Fragment following #
//   query     Query following ?
//
// <url scheme="http">
//   <domain>foo.com</domain>
//   <port>81</port>
//   <path>/x/y/z</path>
//   <query>bar=4&foo=1</query>
//   <fragment>1</fragment>
// </url>
class URLParser
{
private:
  XML::Element& root;

public:
  //--------------------------------------------------------------------------
  // Constructor/Destructor
  URLParser(XML::Element& _root): root(_root) {}
  ~URLParser() {}

  //--------------------------------------------------------------------------
  // Parse a URL string
  // Returns whether successful
  bool parse(const string& url);
};

//==========================================================================
// URL generator (url.cc)
// Generates a URL from an XML structure

// Create the URL parser around a root XML element:
// e.g.
//   XML::Element root("url");
//   Web::URLGenerator urlg(root);
//   string url = urlp.generate();

// The generator takes the same input XML as the URLParser generates, above
class URLGenerator
{
private:
  XML::Element& root;

public:
  //--------------------------------------------------------------------------
  // Constructor/Destructor
  URLGenerator(XML::Element& _root): root(_root) {}
  ~URLGenerator() {}

  //--------------------------------------------------------------------------
  // Generate a URL string
  string generate();
};

//==========================================================================
// HTTP message parser (http-message.cc)
// Reads an HTTP request/response message from the stream
// Also usable for other HTTP-like protocols - e.g. RTSP

// Create the message parser around the root XML element:
// e.g.
//   XML::Element root;
//   Web::HTTPMessageParser hmp(root, cin);
//   hmp.parse();
//   ... use sub-elements of root, e.g. through XPath ...

// the root element is given a tag equal to the request method ('verb'), and
// attributes 'uri' and 'version' from the rest of the first line

// Headers are read into sub-elements of a sub-element <headers> as above

// Body (if any) is read into the content of a sub-element <body> - this
// is safe even if it isn't textual, as long as you don't try to stream
// the XML!

// e.g.
// <POST uri="/foo/register.ccp" version="HTTP/1.0">
//   <headers>
//     <host>www.xmill.com</host>
//     <user-agent>Bizarre Browser</user-agent>
//   </headers>
//   <body>
//     name=fred&id=23453&foo=bar
//   </body>
// </POST>

class HTTPMessageParser
{
private:
  static const unsigned int READ_SIZE = 4096;  
  XML::Element& root;
  istream& in;

public:
  //--------------------------------------------------------------------------
  // Constructor/Destructor
  HTTPMessageParser(XML::Element& _root, istream& _in): 
    root(_root), in(_in) {}

  //--------------------------------------------------------------------------
  // Parse the stream
  // Returns whether successful
  bool parse();
};

//==========================================================================
// HTTP message generator (http-message.cc)
// Generates an HTTP request/response message to a stream
// Also usable for other HTTP-like protocols - e.g. RTSP

// Works as inverse of HTTPMessageParser, above
// Body is only generated if it exists, and a Content-Length header is
// added to suit

class HTTPMessageGenerator
{
private:
  XML::Element& root;
  ostream& out;

public:
  //--------------------------------------------------------------------------
  // Constructor/Destructor
  HTTPMessageGenerator(XML::Element& _root, ostream& _out): 
    root(_root), out(_out) {}

  //--------------------------------------------------------------------------
  // Generate the stream
  // Returns whether successful
  bool generate();
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_WEB_H



