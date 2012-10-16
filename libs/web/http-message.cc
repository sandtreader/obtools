//==========================================================================
// ObTools::Web: http-message.cc
//
// Parser/Generator for HTTP request/response messages
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-web.h"
#include "ot-text.h"
#include <sstream>

namespace ObTools { namespace Web {

//--------------------------------------------------------------------------
// Get a line, or a $ block
// Returns true if read OK - even if blank
bool HTTPMessage::get_first_line(istream& in, string& s)
{
  unsigned int count = 0;

  while (!in.fail())
  {
    int c = in.get();

    switch (c)
    {
      case EOF:  // Error - should end cleanly with blank line
	return false;

      case '\r':
	// Skip remaining LF if present
	c = in.get();
	if (c!=EOF && c!='\n') in.putback(c);
	return true;

      case '\n': // Shouldn't happen, but being liberal for Unix files
	return true;

      case '$':
	if (!count) // first character
	{
	  s+=c;  // Just use this as string
	  return true;
	}
	// Otherwise falling to default...

      default:
	if (++count < MAX_FIRST_LINE)  // DOS protection
	  s+=c;
	else
	  return false;            // Bomb out
    }
  }

  return false;
}


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
    if (!get_first_line(in, line)) return false;
  } while (line.empty());

  // If it's an interleave, read it as a '$' message with code = channel,
  // body = data
  if (line == "$")
  {
    method = "$";
    code = in.get(); // Interleave channel
    if (code==EOF) return false;

    int c1 = in.get();
    int c2 = in.get();
    if (c1==EOF || c2==EOF) return false;

    uint16_t length = (c1 << 8) + c2;

    // Read this many bytes
    body.clear();
    if (length)
    {
      int count = 0;
      while (!in.fail() && count<length)
      {
	char buf[READ_SIZE];
	int wanted = READ_SIZE;
	if (length-count < wanted) wanted = length-count;
	in.read(buf, wanted);
	int got = in.gcount();
	body.append(buf, got);
	count += got;
      }
    }

    url.clear();
    version.clear();
    reason.clear();
    return true;
  }

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

  // Stop now if we just read an interleave packet
  if (method == "$") return true;

  // Only allow read to eof for POST or responses
  // (ugly HTTP-specific hack, but the protocol mixes levels here)
  if (method != "POST" && !method.empty()) read_to_eof = false;

  // Read body - check for Content-Length header and use to limit
  // length if present
  int length = Text::stoi(headers.get("content-length"));

  // Check for chunked encoding
  bool chunked = Text::tolower(headers.get("transfer-encoding")) == "chunked";

  // If no content-length, there are two possibilities:
  //  1) In RTSP, this genuinely has no body and stops here
  //  2) In HTTP, it may have a body up to the end of stream
  //
  // We only allow (2) if we've been explicitly enabled to, otherwise
  // we will break body-less RTSP messages

  body.clear();

  do // Multiple chunks
  {
    if (chunked)
    {
      // Read chunk length (overriding content-length)
      string line;
      if (!MIMEHeaders::getline(in, line)) return false;

      // First line might effectively be blank because it's actually the
      // end of the previous chunk
      if (line.empty() && !MIMEHeaders::getline(in, line)) return false;

      // Split at ;
      vector<string> bits = Text::split(line, ';');
      length = Text::xtoi(bits[0]);
      if (!length) break;  // Last chunk
    }

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
  } while (chunked);

  // If chunked, read optional trailer headers again, including blank line
  // We allow this to fail, because (although unlikely) stream could end here
  if (chunked) headers.read(in);

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
  // Except if chunked transfer encoding set which indicated progressive
  if (!headers.has("Content-Length") && !headers.has("content-length")
   && headers.get("Transfer-Encoding") != "chunked")
    out << "Content-Length: " << body.size() << "\r\n";

  // Output headers
  return headers.write(out);
}

//--------------------------------------------------------------------------
// Write to a stream
// Returns whether successful
bool HTTPMessage::write(ostream &out, bool headers_only) const
{
  // Output headers
  if (!write_headers(out)) return false;

  if (!headers_only)
  {
    // Output body (if any), or size if it's interleaved data
    if (method == "$")
      out << "[Binary data, " << body.size() << " bytes]\n";
    else
      out << body;
  }

  return !out.fail();
}

//------------------------------------------------------------------------
// >> operator to read HTTPMessage from istream
// e.g. cin >> msg;
istream& operator>>(istream& s, HTTPMessage& msg)
{
  msg.read(s);  // !!! Check for failure?
  return s;
}

//------------------------------------------------------------------------
// << operator to write HTTPMessage to ostream
// e.g. cout << msg;
ostream& operator<<(ostream& s, const HTTPMessage& msg)
{
  msg.write(s);
  return s;
}

//--------------------------------------------------------------------------
// Set a cookie with the given optional domain, path and expiry time
void HTTPMessage::set_cookie(const string& name,
                             const string& value,
                             const string& path,
                             const string& domain,
                             Time::Stamp expires)
{
  ostringstream oss;
  // Name value %-encoded, including spaces
  oss << URL::encode(name, false) << "=" << URL::encode(value, false);

  if (!!expires) oss << "; Expires=" << expires.rfc822();
  if (!domain.empty()) oss << "; Domain=" << domain;
  if (!path.empty()) oss << "; Path=" << path;

  headers.put("set-cookie", oss.str());
}

//--------------------------------------------------------------------------
// Get a map of all cookies, name value pairs in values_p
void HTTPMessage::get_cookies(map<string, string>& values_p)
{
  list<string> cookie_headers = headers.get_all("cookie");
  for(list<string>::iterator p = cookie_headers.begin();
      p!=cookie_headers.end();++p)
  {
    string& hdr = *p;
    // Split into pairs on ;
    vector<string> pairs = Text::split(hdr, ';');
    for(vector<string>::iterator q = pairs.begin(); q!=pairs.end(); ++q)
    {
      // Split into name, value on =
      vector<string> bits = Text::split(*q, '=', 2);
      if (bits.size() != 2) continue;
      string name = Text::canonicalise_space(bits[0]);
      string value = Text::canonicalise_space(bits[1]);
      values_p[name] = value;
    }
  }
}

//--------------------------------------------------------------------------
// Get a single cookie value, or empty if not set
string HTTPMessage::get_cookie(const string& name)
{
  map<string, string> values;
  get_cookies(values);
  return values[name];
}

}} // namespaces



