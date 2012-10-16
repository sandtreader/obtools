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

      if (bits.size() == 2)
      {
        // Name=value attributes
        if (aname == "expires")
          expires = Time::Stamp(bits[1]);  // RFC1123 date
        else if (aname == "max-age")
          max_age = Time::Duration(Text::stoi(bits[1]));
        else if (aname == "domain")
          domain = bits[1];
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
    if (!!max_age) oss << "; Max-Age=" << (int)max_age.seconds();
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
// Take cookies from the given server response
void CookieJar::take_cookies_from(const HTTPMessage& response,
                                  const URL& origin)
{
  const list<string> headers = response.headers.get_all("set-cookie");
  string origin_host = origin.get_host();
  MT::RWWriteLock lock(mutex);
  for(list<string>::const_iterator p = headers.begin(); p!=headers.end(); ++p)
  {
    Cookie cookie;
    if (cookie.read_from(*p))
    {
      // Check if domain specified that it is a prefix of the origin
      if (cookie.domain.size())
      {
        if (origin_host.size() < cookie.domain.size()
            || origin_host.compare(0, cookie.domain.size(), cookie.domain))
          continue;
      }

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
  string origin_host = request.url.get_host();
  MT::RWReadLock lock(mutex);
  for(list<Cookie>::const_iterator p = cookies.begin(); p!=cookies.end(); ++p)
  {
    const Cookie& cookie = *p;

    // Check for expiry
    if (!!cookie.expires && now >= cookie.expires) continue;
    if (!!cookie.max_age && now > cookie.created
        && now-cookie.created > cookie.max_age) continue;

    // Check for wrong domain
    // If cookie specifies a domain we use that as a prefix
    if (cookie.domain.size())
    {
      if (origin_host.size() < cookie.domain.size()
          || origin_host.compare(0, cookie.domain.size(), cookie.domain))
        continue;
    }
    else // No domain, must be exact same origin
    {
      if (origin_host != cookie.origin.get_host()) continue;
    }

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
  for(list<Cookie>::iterator p = cookies.begin(); p!=cookies.end(); ++p)
  {
    const Cookie& cookie = *p;

    // Check for expiry
    if ((session_ended && !cookie.expires)
     || (!!cookie.expires && now >= cookie.expires)
     || (!!cookie.max_age && now > cookie.created
         && now-cookie.created > cookie.max_age))
      p = cookies.erase(p);
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



