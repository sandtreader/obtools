//==========================================================================
// ObTools::Web: url.cc
//
// Parser/Generator for URLs
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-web.h"
#include "ot-text.h"
#include <sstream>

namespace ObTools { namespace Web {

//==========================================================================
// URLParser

//--------------------------------------------------------------------------
// Parse a URL string
// Returns whether successful
bool URLParser::parse(const string& url)
{
  string::size_type length = url.size();

  // Extract scheme
  string::size_type p = url.find(':');
  if (p != string::npos)
  {
    if (length < p+3 || url[p+1]!='/' || url[p+2]!='/') return false;
    root.add("scheme", string(url, 0, p));

    // Extract host and optional port
    string::size_type slash = url.find('/', p+3);
    string host;
    if (slash == string::npos)
      host = string(url, p+3);
    else
      host = string(url, p+3, slash-p-3);

    p = host.find(':');
    if (p == string::npos)
      root.add("host", host);
    else
    {
      root.add("host", string(host, 0, p));
      root.add("port", string(host, p+1));
    }

    p = slash;
  }
  else p=0;

  // If no slash, that's it
  if (p == string::npos) return true;

  // Extract path and query
  string::size_type query = url.find('?', p);
  string::size_type hash;
  if (query == string::npos)
  {
    hash = url.find('#', p);
    if (hash == string::npos)
      root.add("path", string(url, p));
    else
      root.add("path", string(url, p, hash-p));
  }
  else
  {
    root.add("path", string(url, p, query-p));
    hash = url.find('#', query+1);
    if (hash == string::npos)
      root.add("query", string(url, query+1));
    else
      root.add("query", string(url, query+1, hash-query-1));
  }

  // Extract hash to end
  if (hash != string::npos)
    root.add("fragment", string(url, hash+1));

  return true;
}

//==========================================================================
// URLGenerator

//--------------------------------------------------------------------------
// Generate a URL string
string URLGenerator::generate()
{
  ostringstream urls;
  XML::XPathProcessor xpath(root);
  string scheme = xpath["scheme"];
  if (scheme.size())
  {
    urls << scheme << "://" << xpath["host"];
    string port = xpath["port"];
    if (port.size()) urls << ":" << port;
  }

  // Now path (all URLs have this)
  urls << xpath["path"];

  // Now optional query...
  string query = xpath["query"];
  if (query.size()) urls << "?" << query;

  // ... and fragment
  string frag = xpath["fragment"];
  if (frag.size()) urls << "#" << frag;

  return urls.str();
}

}} // namespaces



