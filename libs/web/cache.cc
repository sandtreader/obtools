//==========================================================================
// ObTools::Web: cache.cc
//
// HTTP cache implementation
//
// Copyright (c) 2009 ObTools Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-web.h"
#include "ot-text.h"
#include "ot-log.h"
#include <errno.h>

#define DEFAULT_USER_AGENT "ObTools Web Cache"
#define DEFAULT_CACHE_TIME "24 hours"
#define MAX_REDIRECTS 5

namespace ObTools { namespace Web {

//--------------------------------------------------------------------------
// Constructor
Cache::Cache(const File::Directory& _dir, Time::Duration _time,
	     const string& _ua): 
  directory(_dir), cache_time(_time), user_agent(_ua) 
{
  if (!cache_time) cache_time = Time::Duration(DEFAULT_CACHE_TIME);
  if (user_agent.empty()) user_agent = DEFAULT_USER_AGENT;
}

//--------------------------------------------------------------------------
// Fetch a file from the given URL, or from cache
// Returns whether file was fetched, writes file location to path_p if so
bool Cache::fetch(const URL& url, File::Path& path_p)
{
  Log::Streams log;
  log.summary << "Web cache: requesting " << url << endl;

  XML::Element url_xml;
  if (!url.split(url_xml))
  {
    log.error << "Invalid URL: " << url << endl;
    return false;
  }

  XML::XPathProcessor xpath(url_xml);

  // Construct domain directory from URL domain
  File::Directory domain_dir(directory, xpath["host"]);

  // Now is a good time to prune this directory - both for this file
  // and also to get rid of any other junk
  prune(domain_dir);

  // Get filename base from the MD5 of the whole URL
  Misc::MD5 md5;
  string base = md5.sum(url.get_text());

  // Get the extension of the path
  string upath = xpath["path"];
  string::size_type p = upath.rfind('.');
  if (p != string::npos) base += string(upath, p);

  // Construct the filename
  File::Path path(domain_dir, base);
  log.detail << "URL maps to " << path << endl;

  // Does it exist?
  if (path.exists())
  {
    path_p = path;
    return true;
  }

  URL actual_url = url;

  for(int i=0; i<MAX_REDIRECTS; i++)
  {
    log.detail << "Fetch required from " << actual_url << endl;

    // Need to fetch it
    SSL::Context ctx;    // Only used for https
    Web::HTTPClient client(actual_url, &ctx, user_agent, 5, 5);
    HTTPMessage request("GET", actual_url);
    HTTPMessage response;

    if (!client.fetch(request, response))
    {
      log.error << "Fetch from " << actual_url << " failed\n";
      return false;
    }

    switch (response.code)
    {
      case 200:
      {
	log.detail << "File fetched OK, length " << response.body.length() 
		   << endl;

	// Make sure the domain dir exists
	if (!domain_dir.ensure())
	{
	  log.error << "Can't create cache directory " << domain_dir 
		    << ": " << strerror(errno) << endl;
	  return false;
	}

	// Write the file
	string err = path.write_all(response.body);
	if (!err.empty())
	{
	  log.error << "Can't write cache file '" << path 
		    << "': " << err << endl;
	  return false;
	}

	path_p = path;
	return true;
      }

      case 301: case 302:
	actual_url = Web::URL(response.headers.get("location"));
	log.detail << "Redirect to " << actual_url << endl;
	break;  // Loops to retry fetch

      default:
	log.error << "HTTP cache fetch failed: " << response.code 
		  << " " << response.reason << endl;
	return false;
    }
  }

  // Ran out of redirects
  log.error << "Too many redirects from url " << url << endl;
  log.detail << "Last one before we gave up was " << actual_url << endl;
  return false;
}

//--------------------------------------------------------------------------
// Fetch an object from the given URL, or from cache, as a string
// Returns whether file was fetched, writes file contents to contents_p if so
bool Cache::fetch(const URL& url, string& contents_p)
{
  File::Path path;
  return fetch(url, path) && path.read_all(contents_p);
}

//--------------------------------------------------------------------------
// Prune any out-of-date files
void Cache::prune(File::Directory &dir)
{
  Time::Stamp now = Time::Stamp::now();
  list<File::Path> paths;
  dir.inspect(paths);
  for(list<File::Path>::iterator p=paths.begin(); p!=paths.end(); ++p)
  {
    File::Path& path = *p;
    Time::Stamp last(path.last_modified());
    if (now>last && now-last>cache_time)
    {
      Log::Streams log;
      log.detail << path << " is out of date - deleting\n";
      path.erase();
    }
  }
}

}} // namespaces



