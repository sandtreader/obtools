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

//--------------------------------------------------------------------------
// Read from a stream
// Returns whether successful
bool HTTPMessage::read(istream &in)
{
  // Read first line
  string line;
  if (!MIMEHeaders::getline(in, line)) return false;

  // Split line into method, URI and version, and set in root
  string::size_type sp1 = line.find(' ');
  if (sp1 == string::npos) return false;
  string first(line, 0, sp1);

  // Check for '/' in first word, indicating response
  if (first.find('/') != string::npos)
  {
    // It's a response - get first word as version
    version = first;

    // Next word is code
    string::size_type sp2 = line.find(' ', sp1+1);
    if (sp2 == string::npos) return false;
    code = atoi(string(line, sp1+1, sp2-sp1-1).c_str());

    // Rest is reason 
    reason = string(line, sp2+1);
  }
  else
  {
    // It's a request
    method = first;

    // URI is next
    string::size_type sp2 = line.find(' ', sp1+1);
    if (sp2 == string::npos) return false;
    url.text = string(line, sp1+1, sp2-sp1-1);

    // Version is the rest
    version = string(line, sp2+1);
  }

  // Now read headers
  if (!headers.read(in)) return false;

  // Finally read body - check for Content-Length header and use to limit 
  // length if present
  int length = atoi(headers.get("content-length").c_str());
  
  // !!! For now, assume lack of content-length means no body at all!
  // This is true of RTSP, but not HTTP in general
  // This is potentially protocol-specific - some methods never have bodies,
  // some can have.  Old HTTP clients implementing POST may not provide
  // a Content-Length!

  // Note: Code to handle read-to-end-of-stream meaning of length=0
  // is still in here
  if (length)
  {
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
      body.append(buf, got);
      count += got;
    }
  }

  return true;
}

//--------------------------------------------------------------------------
// Write to a stream
// Returns whether successful
bool HTTPMessage::write(ostream &out) const
{
  // Check stream is OK
  if (out.fail()) return false;

  // Check for request or response
  if (method.size())
  {
    // Request
    out << method << ' ' << url << ' ' << version << "\r\n";
  }
  else
  {
    // Response
    out << version << ' ' << code << ' ' << reason << "\r\n";
  }

  // Check for a content-length header, and if not present, add one
  if (!body.empty() && !headers.has("content-length"))
    out << "Content-length: " << body.size() << "\r\n";

  // Output headers
  if (!headers.write(out)) return false;

  // Output body (if any)
  out << body;

  return !out.fail();
}

//------------------------------------------------------------------------
// >> operator to read HTTPMessage from istream
// e.g. cin >> url;
istream& operator>>(istream& s, HTTPMessage& msg)
{
  msg.read(s);  // !!! Check for failure?
  return s;
}

//------------------------------------------------------------------------
// << operator to write HTTPMessage to ostream
// e.g. cout << url;
ostream& operator<<(ostream& s, const HTTPMessage& msg)
{
  msg.write(s);
  return s;
}

}} // namespaces



