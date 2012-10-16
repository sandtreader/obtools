//==========================================================================
// ObTools::Web: url.cc
//
// Representation of URL, and split/combine to/from XML
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
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
      urls << "//";

      // Add username, password if set
      string user = xpath["user"];
      if (user.size())
      {
	urls << user;
	string password = xpath["password"];
	if (password.size()) urls << ":" << password;
	urls << '@';
      }

      urls << host;

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
bool URL::split(XML::Element& xml) const
{
  // Set XML element name so it will print OK
  xml.name = "url";

  string::size_type length = text.size();

  // Extract scheme - must be before / (query may contain a colon)
  string::size_type p = text.find(':');
  string::size_type q = text.find('/');
  if (p != string::npos && (q==string::npos || p<q))
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

      // Check for @ prefix - username and password
      p = host.find('@');
      if (p != string::npos)
      {
	// We have user:pass@host
	string::size_type r = host.find(':');
	if (r<p)  // Note, don't trip over later : for port number
	{
	  // Have password
	  xml.add("user", string(host, 0, r));
	  xml.add("password", string(host, r+1, p-r-1));
	}
	else
	{
	  // Just user
	  xml.add("user", string(host, 0, p));
	}

	// Remove from host
	host = string(host, p+1);
      }

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
string URL::get_path() const
{
  XML::Element xml;
  if (!split(xml)) return "";
  return xml.get_child("path").content;
}

//------------------------------------------------------------------------
// Quick access to query of URL
// Returns query or "" if can't read it
string URL::get_query() const
{
  XML::Element xml;
  if (!split(xml)) return "";
  return xml.get_child("query").content;
}

//------------------------------------------------------------------------
// Get query as a property list
// Returns whether query was available, fills props if so
// Note: Handles + for space and % decode in values
bool URL::get_query(Misc::PropertyList& props) const
{
  string query = get_query();
  if (query.empty()) return false;

  // %-decode it, with +
  decode(query, props, true);
  return true;
}

//------------------------------------------------------------------------
// Quick access to fragment of URL
// Returns fragment or "" if can't read it
string URL::get_fragment() const
{
  XML::Element xml;
  if (!split(xml)) return "";
  return xml.get_child("fragment").content;
}

//------------------------------------------------------------------------
// << operator to write URL to ostream
// e.g. cout << url;
ostream& operator<<(ostream& s, const URL& u)
{
  s<<u.text;
  return s;
}

//------------------------------------------------------------------------
// Static function to URL-encode (percent-encode) a string
// Escapes space as '+' if 'space_as_plus' is set (the default)
string URL::encode(const string& s, bool space_as_plus)
{
  string r;
  string::size_type l = s.size();
  char buf[4];

  for(string::size_type p=0; p<l; ++p)
  {
    unsigned char c = s[p];

    // Check for controls and 8-bit
    if (c<32 || c>126)
    {
      snprintf(buf, 4, "%%%02X", (unsigned int)c);
      r+=buf;
    }
    else switch (c)
    {
      case ' ':
	if (space_as_plus)
	  r+='+';
	else
	  r+="%20";
	break;

      // Reserved characters;
      case '+': case '%':
      case '!': case '*': case '\'': case '(': case ')':
      case ';': case ':': case '@': case '&': case '=':
      case '$': case ',': case '/': case '?': case '#':
      case '[': case ']':
	snprintf(buf, 4, "%%%02X", (unsigned int)c);
	r+=buf;
	break;

      // Normal characters
      default:
	r += c;
    }
  }

  return r;
}

//------------------------------------------------------------------------
// Static function to URL-encode (percent-encode) a set of variables
// (expressed as a PropertyList)
// Escapes space as '+' if 'space_as_plus' is set (the default)
string URL::encode(const Misc::PropertyList& props, bool space_as_plus)
{
  string r;
  for(Misc::PropertyList::const_iterator p=props.begin(); p!=props.end(); ++p)
  {
    if (r.size()) r+='&';
    r+=encode(p->first, space_as_plus);
    r+='=';
    r+=encode(p->second, space_as_plus);
  }

  return r;
}

//------------------------------------------------------------------------
// Static function to URL-decode (percent-encode) a string
// Decodes '+' as space if 'space_as_plus' is set (the default)
string URL::decode(const string& s, bool space_as_plus)
{
  string r;
  string::size_type l = s.size();

  for(string::size_type p=0; p<l; ++p)
  {
    char c = s[p];

    switch (c)
    {
      case '+':
	if (space_as_plus)
	  r += ' ';
	else
	  r += '+';
	break;

      case '%':
      {
	string hex;
	if (++p < l) hex+=s[p];
	if (++p < l) hex+=s[p];
	r += (char)Text::xtoi(hex);
	break;
      }

      default:
	r += c;
    }
  }

  return r;
}

//------------------------------------------------------------------------
// Static function to URL-decode (percent-encode) a multi-valued string
// (x-www-form-urlencoded) into a property list
// Decodes '+' as space if 'space_as_plus' is set (the default)
void URL::decode(const string& s, Misc::PropertyList& props,
		 bool space_as_plus)
{
  // Split on &
  vector<string> params = Text::split(s, '&', false);
  for(vector<string>::iterator p = params.begin(); p!=params.end(); ++p)
  {
    string& param = *p;
    size_t q = param.find('=');
    if (q && q != string::npos)
    {
      string name(param, 0, q);
      string value(param, q+1);
      props.add(name, decode(value, space_as_plus));
    }
  }
}

}} // namespaces



