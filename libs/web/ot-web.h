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
#include "ot-misc.h"

#if !defined(_SINGLE)
#include "ot-mt.h"
#endif

namespace ObTools { namespace Web { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// URL (url.cc)
// Represents a Web URL
//
// Converts to and from an XML format representing the URL structure:
// The XML element has sub-elements (NOT attributes) as follows:
//   scheme    Scheme (e.g. http) - lower cased
//   host      Host/domain (if present) - lower cased
//   port      Port number (if present)
//   path      URL path (including '/' if absolute) (as specified)
//   fragment  Fragment following #
//   query     Query following ?

class URL
{
public:
  string text;

  //--------------------------------------------------------------------------
  // Constructors
  URL() {}
  URL(const string& s): text(s) {}

  //--------------------------------------------------------------------------
  // Constructor from XML format
  // Pass in any element with above sub-elements
  URL(XML::Element& xml);

  //--------------------------------------------------------------------------
  // Split text into XML
  // Pass in an empty named element, and this will fill it up as above
  // Returns whether successful (valid URL)
  bool split(XML::Element& xml);
};

//------------------------------------------------------------------------
// << operator to write URL to ostream
// e.g. cout << url;
ostream& operator<<(ostream& s, const URL& u);

//==========================================================================
// MIME header block (mime-headers.cc)
// Represents a block of MIME headers - e.g. from a mail message, or an HTTP
// request.  
//
// Stores in XML, since we have convenient access & iteration methods for this.
//
// On input, headers are unfolded and each header is added as a sub-element of
// the root 'xml', with name (tag) equal to the header tag **lowercased**, 
// and content equal to the text of the header
//
// On output, header names are generated with first letter capitalised, 
// following convention.  Values longer than 64 characters are folded at 
// commas (if any) or spaces (if any)             

class MIMEHeaders
{
private:
  static const unsigned int MAX_HEADER = 8000;  // Input DoS protection
  static const unsigned int MAX_LINE   = 60;    // For output folding

public:
  XML::Element xml;

  //--------------------------------------------------------------------------
  // Constructor
  MIMEHeaders(): xml("headers") {}

  //--------------------------------------------------------------------------
  // Check for presence of a header
  bool has(const string& name) const
  { return !!xml.get_child(name); }

  //--------------------------------------------------------------------------
  // Get a specific header (first of that name)
  string get(const string& name) const
  { return xml.get_child(name).content; }

  //--------------------------------------------------------------------------
  // Insert a header
  void put(const string& name, const string& value)
  { xml.add(name, value); }

  //--------------------------------------------------------------------------
  // Split multi-value headers at commas
  // Reads all headers of name 'name', and splits any with commas at
  // comma to give a flattened list of values
  list<string> get_all(const string& name);

  //--------------------------------------------------------------------------
  // Split a header value (e.g. from get or get_all) into a prime value
  // and parameters delineated by ';'
  // Parameters without a value are given the value '1'
  // The value is modified to be without parameters

  // Useful for Content-Type (HTTP), Transport (RTSP) etc.
  // e.g.
  //   Content-Type: text/html; charset=ISO-8859-1; pure
  //
  // Leaves 'text/html' in value, and property list:
  //   charset     ISO-8859-1
  //   pure        1
  static Misc::PropertyList split_parameters(string& value);

  //--------------------------------------------------------------------------
  // Parse headers from a stream
  // Returns whether successful
  // Skips the blank line delimiter, leaving stream ready to read message
  // body (if any)
  bool read(istream& in);

  //--------------------------------------------------------------------------
  // Generates headers to a stream
  // Returns whether successful (can only fail if stream fails)
  // Includes the blank line delimiter, leaving stream ready to write message
  // body (if any)
  bool write(ostream& out) const;

  //--------------------------------------------------------------------------
  // Get a line from a stream
  // Returns true if read OK - even if blank
  // Exported for the convenience of HTTP reader - see below
  static bool getline(istream& in, string& s);
};

//------------------------------------------------------------------------
// >> operator to read MIMEHeaders from istream
// e.g. cin >> url;
istream& operator>>(istream& s, MIMEHeaders& mh);

//------------------------------------------------------------------------
// << operator to write MIMEHeaders to ostream
// e.g. cout << url;
ostream& operator<<(ostream& s, const MIMEHeaders& mh);

//==========================================================================
// HTTP message (http-message.cc)
// Represents an HTTP request or response message
// Also usable for other HTTP-like protocols - e.g. RTSP
// Note we represent both request and response in the same structure because
// of bidirectional protocols like RTSP where either client or server might
// receive either at any time
// (We distinguish responses by virtue of the first word containing a '/')

// For requests, members 'method' and 'uri' are set
// For responses, members 'code' and 'reason' are set
// Use the presence of 'method' to distinguish
// Version is always set
// Headers are always read into the 'headers' MIMEHeaders structure
// Body (if any) is read into 'body'

class HTTPMessage
{
private:
  static const unsigned int READ_SIZE = 4096;  

public:
  // Request fields
  string method;
  URL url;

  // Response fields
  int code;
  string reason;

  // Shared fields
  string version;
  MIMEHeaders headers;
  string body;

  //--------------------------------------------------------------------------
  // Basic constructor
  HTTPMessage() {}

  //--------------------------------------------------------------------------
  // Constructor for requests
  HTTPMessage(const string& _method, const string& _url, 
	      const string& _version = "HTTP/1.0"):
    method(_method), url(_url), version(_version) {}

  //--------------------------------------------------------------------------
  // Constructor for responses
  HTTPMessage(int _code, const string& _reason,
	      const string& _version = "HTTP/1.0"):
    code(_code), reason(_reason), version(_version) {}

  //--------------------------------------------------------------------------
  // Check for request (not response)
  bool is_request() { return !method.empty(); }

  //--------------------------------------------------------------------------
  // Read from a stream
  // Returns whether successful
  bool read(istream &in);

  //--------------------------------------------------------------------------
  // Write to a stream
  // Returns whether successful
  bool write(ostream &out) const;
};

//------------------------------------------------------------------------
// >> operator to read HTTPMessage from istream
// e.g. cin >> url;
istream& operator>>(istream& s, HTTPMessage& msg);

//------------------------------------------------------------------------
// << operator to write HTTPMessage to ostream
// e.g. cout << url;
ostream& operator<<(ostream& s, const HTTPMessage& msg);

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_WEB_H



