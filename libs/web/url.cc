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
  // Set XML element name so it will print OK
  xml.name = "url";

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
// Quick access to path of URL
// Returns path or "" if can't read it
string URL::get_path()
{
  XML::Element xml;
  if (!split(xml)) return "";
  return xml.get_child("path").content;
}

//------------------------------------------------------------------------
// Quick access to query of URL
// Returns query or "" if can't read it
string URL::get_query()
{
  XML::Element xml;
  if (!split(xml)) return "";
  return xml.get_child("query").content;
}

//------------------------------------------------------------------------
// Get query as a property list
// Returns whether query was available, fills props if so
// Note: Handles + for space and % decode in values
bool URL::get_query(Misc::PropertyList& props)
{
  string query = get_query();
  if (query.empty()) return false;

  // Split on &
  vector<string> params = Text::split(query, '&', false);
  for(vector<string>::iterator p = params.begin(); p!=params.end(); ++p)
  {
    string& param = *p;
    size_t q = param.find('=');
    if (q && q != string::npos)
    {
      string name(param, 0, q);
      string value;

      // Fix up the value
      for(++q; q<param.size(); ++q)
      {
	char c = param[q];
	switch (c)
	{
	  case '+':
	    value += ' ';
	    break;

	  case '%':
	  {
	    string hex;
	    if (++q < param.size()) hex+=param[q];
	    if (++q < param.size()) hex+=param[q];
	    value += (char)Text::xtoi(hex);
	    break;
	  }

	  default:
	    value += c;
	}
      }

      props.add(name, value);
    }
  }
  
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



