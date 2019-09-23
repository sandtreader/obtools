//==========================================================================
// ObTools::Web: cookies.cc
//
// HTTP client cookie jar
//
// Copyright (c) 2012 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-web.h"
#include "ot-text.h"
#include <sstream>

namespace ObTools { namespace Web {

//==========================================================================
// Cookie structure

//--------------------------------------------------------------------------
// Read from a Set-Cookie header value
// Returns whether valid cookie read
bool Cookie::read_from(const string& header_value)
{
  // Split into pairs on ;
  vector<string> pairs = Text::split(header_value, ';', true);
  for(vector<string>::iterator q = pairs.begin(); q!=pairs.end(); ++q)
  {
    // Split into name, value on =
    vector<string> bits = Text::split(*q, '=', true, 2);

    // First one is special, holds the actual cookie
    if (q == pairs.begin())
    {
      if (bits.size() != 2) return false;
      if (!bits[0].size()) return false;
      name = bits[0];
      value = bits[1]; // can be empty

      // Check for quotes and remove
      if (value.size() > 2 && value[0] == '"' && value[value.size()-1] == '"')
        value = string(value, 1, value.size()-2);
    }
    else
    {
      // Attributes
      string aname = Text::tolower(bits[0]);

      if (bits.size() == 2 && bits[1].size())
      {
        // Name=value attributes
        if (aname == "expires")
          expires = Time::Stamp(bits[1]);  // RFC1123 date
        else if (aname == "max-age")
        {
          int delta = Text::stoi(bits[1]);
          if (delta > 0)
            expires = Time::Stamp::now()+Time::Duration(delta);
          else
            expires = Time::Stamp(1);  // Force expiry
        }
        else if (aname == "domain")
        {
          domain = Text::tolower(bits[1]);
          if (domain[0] == '.') domain.erase(0, 1);  // Strip leading .
        }
        else if (aname == "path")
          path = bits[1];
      }
      else
      {
        // Single-word boolean attributes
        if (aname == "secure")
          secure = true;
        else if (aname == "httponly")
          http_only = true;
      }
    }
  }

  return true;
}

//--------------------------------------------------------------------------
// Output as a string, including attributes if attrs is set
string Cookie::str(bool attrs) const
{
  ostringstream oss;
  oss << name << "=" << value;
  if (attrs)
  {
    if (!!expires) oss << "; Expires=" << expires.rfc822();
    if (domain.size()) oss << "; Domain=" << domain;
    if (path.size()) oss << "; Path=" << path;
    if (secure) oss << "; Secure";
    if (http_only) oss << "; HttpOnly";
  }

  return oss.str();
}

//==========================================================================
// Cookie jar

//--------------------------------------------------------------------------
// Internals
namespace
{
  // Domain suffix match
  bool domain_match(const string& cookie_domain, const string& origin_host)
  {
    string::size_type c_size = cookie_domain.size();
    string::size_type o_size = origin_host.size();

    // Origin must be either identical or domain must be a suffix of it
    // with '.' as the previous character
    return origin_host == cookie_domain
      || (o_size > c_size
          && !origin_host.compare(o_size-c_size, c_size, cookie_domain)
          && origin_host[o_size-c_size-1] == '.');
  }

  // Path prefix match
  bool path_match(const string& cookie_path, const string& origin_path)
  {
    string::size_type c_size = cookie_path.size();
    string::size_type o_size = origin_path.size();

    // Origin must either be identical or cookie path must be a prefix of it
    // either ending with '/' or with '/' as the next character
    return (!origin_path.compare(0, c_size, cookie_path)
            && (o_size == c_size
                || (o_size > c_size
                    && (origin_path[c_size-1] == '/'
                        || origin_path[c_size] == '/'))));
  }
}

//--------------------------------------------------------------------------
// Take cookies from the given server response
void CookieJar::take_cookies_from(const HTTPMessage& response,
                                  const URL& origin)
{
  const list<string> headers = response.headers.get_all("set-cookie");
  XML::Element origin_xml;
  if (!origin.split(origin_xml)) return;
  string origin_scheme = Text::tolower(origin_xml.get_child("scheme").content);
  string origin_host = Text::tolower(origin_xml.get_child("host").content);
  string origin_path = Text::tolower(origin_xml.get_child("path").content);

  MT::RWWriteLock lock(mutex);
  for(list<string>::const_iterator p = headers.begin(); p!=headers.end(); ++p)
  {
    Cookie cookie;
    if (cookie.read_from(*p))
    {
      // Check if domain specified that it is a suffix of the origin
      if (cookie.domain.size() && !domain_match(cookie.domain, origin_host))
        continue;

      // Check if path specified and absolute - if not, ignore it and get
      // path from origin
      if (!cookie.path.size() || cookie.path[0] != '/')
      {
        cookie.path = origin_path;

        // Get default path
        if (!cookie.path.size() || cookie.path[0] != '/')
          cookie.path = "/";
        else
        {
          string::size_type slash = cookie.path.rfind('/');
          if (slash != string::npos)
            cookie.path.erase(slash?slash:1); // / if only one
        }
      }

      // If cookie is HTTPOnly check the scheme is HTTP
      if (cookie.http_only && origin_scheme != "http"
                           && origin_scheme != "https") continue;

      // Evict any existing identical ones (name, domain, path)
      cookies.remove(cookie);

      // Add new one
      cookie.created = Time::Stamp::now();
      cookie.origin = origin;
      cookies.push_back(cookie);
    }
  }
}

//--------------------------------------------------------------------------
// Add cookies to the given client request
void CookieJar::add_cookies_to(HTTPMessage& request)
{
  string s;
  Time::Stamp now = Time::Stamp::now();

  XML::Element origin_xml;
  if (!request.url.split(origin_xml)) return;
  string origin_scheme = Text::tolower(origin_xml.get_child("scheme").content);
  string origin_host = Text::tolower(origin_xml.get_child("host").content);
  string origin_path = Text::tolower(origin_xml.get_child("path").content);
  if (origin_path.empty()) origin_path = "/";

  MT::RWReadLock lock(mutex);
  for(list<Cookie>::const_iterator p = cookies.begin(); p!=cookies.end(); ++p)
  {
    const Cookie& cookie = *p;

    // Check for expiry
    if (!!cookie.expires && now >= cookie.expires) continue;

    // Check for wrong domain
    if (cookie.domain.size())
    {
      // If cookie specifies a domain we allow suffix match
      if (!domain_match(cookie.domain, origin_host)) continue;
    }
    else
    {
      // No domain, must be exact same origin
      if (origin_host != cookie.origin.get_host()) continue;
    }

    // Check for wrong path
    if (!path_match(cookie.path, origin_path)) continue;

    // Check for security
    if (cookie.secure && origin_scheme != "https") continue;

    // Check for http only
    if (cookie.http_only && origin_scheme != "http"
                         && origin_scheme != "https") continue;

    if (s.size()) s += "; ";
    s += cookie.str();
  }

  if (s.size()) request.headers.put("cookie", s);
}

//--------------------------------------------------------------------------
// Prune expired cookies from the jar, including session cookies if session
// ended
void CookieJar::prune(bool session_ended)
{
  Time::Stamp now = Time::Stamp::now();
  MT::RWWriteLock lock(mutex);
  for(list<Cookie>::iterator p = cookies.begin(); p!=cookies.end();)
  {
    const Cookie& cookie = *p;

    // Check for expiry
    if ((session_ended && !cookie.expires)
     || (!!cookie.expires && now >= cookie.expires))
      p = cookies.erase(p);
    else
      ++p;
  }
}

//--------------------------------------------------------------------------
// Dump the cookie jar to the given stream
void CookieJar::dump(ostream& sout)
{
  MT::RWReadLock lock(mutex);
  for(list<Cookie>::const_iterator p = cookies.begin(); p!=cookies.end(); ++p)
  {
    const Cookie& cookie = *p;
    sout << "  " << cookie.str(true) << endl;
  }
}

}} // namespaces



