//==========================================================================
// ObTools::Web: http-message.cc
//
// Parser/Generator for HTTP request/response messages
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-web.h"
#include "ot-text.h"
#include <sstream>

namespace ObTools { namespace Web {

//==========================================================================
// HTTPMessageParser

//--------------------------------------------------------------------------
// Parse a message
// Returns whether successful
bool HTTPMessageParser::parse()
{
  // Create <header> sub-element for MIME header parser
  XML::Element& headers = root.add("headers");

  // Build MIME header parser
  MIMEHeaderParser mhp(headers, in);

  // Read first line
  string line;
  if (!mhp.getline(line)) return false;

  // Split line into method, URI and version, and set in root
  string::size_type sp1 = line.find(' ');
  if (sp1 == string::npos) return false;
  root.name = string(line, 0, sp1);

  // URI
  string::size_type sp2 = line.find(' ', sp1+1);
  if (sp2 == string::npos) return false;
  string uri(string(line, sp1+1, sp2-sp1-1));
  root.set_attr("uri", uri);

  // Split URI into URL subelement
  XML::Element& url = root.add("url");
  URLParser urlp(url);
  if (!urlp.parse(uri)) return false;

  // Version
  root.set_attr("version", string(line, sp2+1));

  // Now read headers
  if (!mhp.parse()) return false;

  // Finally read body - check for Content-Length header and use to limit 
  // length if present
  XML::XPathProcessor xpath(headers);
  int length = xpath.get_value_int("content-length", 0);
  
  // !!! For now, assume lack of content-length means no body at all!
  // This is potentially protocol-specific - some methods never have bodies,
  // some can have.  Old HTTP clients implementing POST may not provide
  // a Content-Length!

  // Note: Code to handle read-to-end-of-stream meaning of length=0
  // is still in here
  if (length)
  {
    // Create <body> sub-element
    XML::Element& body = root.add("body");

    // Try to read this much or up to end of stream
    int count = 0;
    while (!in.fail() && (!length || count<length))
    {
      char buf[READ_SIZE];
      int wanted = READ_SIZE;
      if (length && length-count < wanted) wanted = length-count;

      in.read(buf, wanted);
      int got = in.gcount();

      if (length && wanted != got) return false;  // Failed early

      // Add to body content
      body.content.append(buf, got);
      count += got;
    }
  }

  return true;
}

//==========================================================================
// HTTPMessageGenerator

//--------------------------------------------------------------------------
// Generate a message
// Returns whether successful
bool HTTPMessageGenerator::generate()
{
  // Check stream is OK
  if (out.fail()) return false;

  // Output first line
  out << root.name << ' ' << root["uri"] << ' ' << root["version"] << "\r\n";

  // If body exists, add content-length header
  XML::Element& headers = root.get_child("headers");
  XML::Element& body = root.get_child("body");
  if (body.valid())
  {
    ostringstream oss;
    oss << body.content.size();
    headers.add("content-length", oss.str());
  }

  // Output headers
  MIMEHeaderGenerator mhg(headers, out);
  if (!mhg.generate()) return false;
  
  // Output body (if any)
  if (body.valid()) out << body.content;

  return !out.fail();
}

}} // namespaces



