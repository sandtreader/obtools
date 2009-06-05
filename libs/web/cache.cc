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
#define MAX_REDIRECTS 5
#define STATUS_PREFIX "."
#define STATUS_SUFFIX ".status.xml"

namespace ObTools { namespace Web {

//--------------------------------------------------------------------------
// Constructor
  Cache::Cache(const File::Directory& _dir, const string& _ua): 
  directory(_dir), user_agent(_ua) 
{
  if (user_agent.empty()) user_agent = DEFAULT_USER_AGENT;
}

//--------------------------------------------------------------------------
// Fetch a file from the given URL, or from cache
// max_age gives maximum time since last update check before doing another
// If zero (the default) no updates are done
// Returns whether file is available, writes file location to path_p if so
bool Cache::fetch(const URL& url, File::Path& path_p, Time::Duration max_age)
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

  // Construct the status filename
  File::Path status_path(domain_dir, STATUS_PREFIX+base+STATUS_SUFFIX);
  XML::Configuration status_cfg(status_path.str(), log.error);
  if (status_path.exists())
    status_cfg.read("status");  // Read it
  else
    status_cfg.replace_root("status"); // Create root

  // Get last check time
  if (!!max_age)
  {
    Time::Stamp last_check = Time::Stamp(status_cfg["check/@time"]);
    if (!!last_check)
      log.detail << "Last checked at " << last_check.iso() << endl;
    else
      log.detail << "This is the first check\n";

    // Does it need checking again?
    if (Time::Stamp::now() - last_check < max_age)
    {
      log.detail << "Doesn't need checking again until " 
		 << (last_check+max_age).iso() << endl;
      max_age = Time::Duration();
    }
  }

  // If no update check required, and it exists, that's enough
  if (!max_age && path.exists())
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

    // If we have existing last-modified and/or etag, make it conditional
    string lm = status_cfg["server/last-modified"];
    if (!lm.empty()) request.headers.put("If-Modified-Since", lm);

    string etag = status_cfg["server/etag"];
    if (!etag.empty()) request.headers.put("If-None-Match", etag);

    // Do it
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

	// Capture last-modified and E-tag for the config
	status_cfg.ensure_path("server/last-modified");
	status_cfg.set_value("server/last-modified", 
			     response.headers.get("last-modified"));

	status_cfg.ensure_path("server/etag");
	status_cfg.set_value("server/etag", response.headers.get("etag"));

	// Update last check time
	status_cfg.ensure_path("check");
	status_cfg.set_value("check/@time", Time::Stamp::now().iso()); 

	status_cfg.write();

	path_p = path;
	return true;
      }

      case 301: case 302:  // Moved
	actual_url = Web::URL(response.headers.get("location"));
	log.detail << "Redirect to " << actual_url << endl;
	break;  // Loops to retry fetch

      case 304:  // Not modified
	// Update last check time only
	status_cfg.ensure_path("check");
	status_cfg.set_value("check/@time", Time::Stamp::now().iso()); 
	status_cfg.write();
	path_p = path;
	return true;

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
// Returns whether file is available, writes file contents to contents_p if so
bool Cache::fetch(const URL& url, string& contents_p, Time::Duration max_age)
{
  File::Path path;
  return fetch(url, path, max_age) && path.read_all(contents_p);
}


}} // namespaces



