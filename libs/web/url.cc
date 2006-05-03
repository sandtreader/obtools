//==========================================================================
// ObTools::Web: url.cc
//
// Representation of URL, and split/combine to/from XML
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-web.h"
#include "ot-text.h"
#include <sstream>

namespace ObTools { namespace Web {

//--------------------------------------------------------------------------
// Constructor from XML format
URL::URL(XML::Element& xml)
{
  ostringstream urls;
  XML::XPathProcessor xpath(xml);
  string scheme = xpath["scheme"];
  if (scheme.size())
  {
    urls << scheme << ":";

    string host = xpath["host"];
    if (host.size())
    {
      urls << "//" << host;
      
      string port = xpath["port"];
      if (port.size()) urls << ":" << port;
    }
  }

  // Now path (all URLs have this)
  urls << xpath["path"];

  // Now optional query...
  string query = xpath["query"];
  if (query.size()) urls << "?" << query;

  // ... and fragment
  string frag = xpath["fragment"];
  if (frag.size()) urls << "#" << frag;

  text = urls.str();
}

//--------------------------------------------------------------------------
// Split text into XML
bool URL::split(XML::Element& xml)
{
  string::size_type length = text.size();

  // Extract scheme
  string::size_type p = text.find(':');
  if (p != string::npos)
  {
    xml.add("scheme", string(text, 0, p));

    // Get host and port if specified
    if (length > p+2 && text[p+1]=='/' && text[p+2]=='/')
    {
      // Check for empty host
      if  (length == p+3) return false;

      string::size_type slash = text.find('/', p+3);
      string host;
      if (slash == string::npos)
	host = string(text, p+3);
      else
	host = string(text, p+3, slash-p-3);

      p = host.find(':');
      if (p == string::npos)
	xml.add("host", host);
      else
      {
	xml.add("host", string(host, 0, p));
	xml.add("port", string(host, p+1));
      }

      p = slash;
    }
    else
    {
      // No //host given - technically invalid, but we'll allow it to
      // handle (e.g.) misimplemented RTSP horrors like rtsp:/media.mpg

      // Skip over colon
      p++;
    }
  }
  else p=0;

  // If no slash, that's it
  if (p == string::npos) return true;

  // If nothing left, that's it
  if (p >= length) return true;

  // Extract path and query
  string::size_type query = text.find('?', p);
  string::size_type hash;
  if (query == string::npos)
  {
    hash = text.find('#', p);
    if (hash == string::npos)
      xml.add("path", string(text, p));
    else
      xml.add("path", string(text, p, hash-p));
  }
  else
  {
    xml.add("path", string(text, p, query-p));
    hash = text.find('#', query+1);
    if (hash == string::npos)
      xml.add("query", string(text, query+1));
    else
      xml.add("query", string(text, query+1, hash-query-1));
  }

  // Extract hash to end
  if (hash != string::npos)
    xml.add("fragment", string(text, hash+1));

  return true;
}

//------------------------------------------------------------------------
// << operator to write URL to ostream
// e.g. cout << url;
ostream& operator<<(ostream& s, const URL& u) 
{ 
  s<<u.text; 
  return s; 
}

}} // namespaces



