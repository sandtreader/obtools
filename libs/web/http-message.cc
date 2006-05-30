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
// Read request/response and headers from a stream
// Leave stream ready to read body (if any)
// Returns whether successful
bool HTTPMessage::read_headers(istream &in)
{
  // Read first line - be lenient about blank lines, as per RFC
  string line;
  do
  {
    if (!MIMEHeaders::getline(in, line)) return false;
  } while (line.empty());
  
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

    // Method, URL are empty
    method.clear();
    url.clear();
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

    // Code, reason are empty
    code = 0;
    reason.clear();
  }

  // Now read headers
  return headers.read(in);
}

//--------------------------------------------------------------------------
// Read from a stream
// Returns whether successful
bool HTTPMessage::read(istream &in, bool read_to_eof)
{
  // Read start line & headers
  if (!read_headers(in)) return false;

  // Only allow read to eof for POST (ugly HTTP-specific hack, but
  // the protocol mixes levels here)
  if (method != "POST") read_to_eof = false;

  // Read body - check for Content-Length header and use to limit 
  // length if present
  int length = Text::stoi(headers.get("content-length"));

  // If no content-length, there are two possibilities:
  //  1) In RTSP, this genuinely has no body and stops here
  //  2) In HTTP, it may have a body up to the end of stream
  //
  // We only allow (2) if we've been explicitly enabled to, otherwise
  // we will break body-less RTSP messages

  body.clear();
  if (length || read_to_eof)
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

      // Add to body content
      body.append(buf, got);
      count += got;
    }
  }

  return true;
}

//--------------------------------------------------------------------------
// Write request/response and headers to a stream
// Returns whether successful
bool HTTPMessage::write_headers(ostream &out) const
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
  return headers.write(out);
}

//--------------------------------------------------------------------------
// Write to a stream
// Returns whether successful
bool HTTPMessage::write(ostream &out) const
{
  // Output headers
  if (!write_headers(out)) return false;

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



