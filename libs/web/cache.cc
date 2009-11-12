//==========================================================================
// ObTools::Web: cache.cc
//
// HTTP cache implementation
//
// Copyright (c) 2009 ObTools Limited.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
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
Cache::Cache(const File::Directory& _dir, SSL::Context *_ssl_ctx,
	     const string& _ua): 
  directory(_dir), ssl_ctx(_ssl_ctx), user_agent(_ua) 
{
  if (user_agent.empty()) user_agent = DEFAULT_USER_AGENT;
}

//--------------------------------------------------------------------------
// Fetch a file from the given URL, or from cache
// If check_for_updates is set, uses conditional GET to check whether a
// new version exists, if the item's update-time has passed since last check
// Returns whether file is available, writes file location to path_p if so
bool Cache::fetch(const URL& url, File::Path& path_p, bool check_for_updates)
{
  Log::Streams log;
  log.summary << "Web cache: requesting " << url << endl;

  File::Directory domain_dir;
  File::Path file_path, status_path;
  if (!get_paths(url, domain_dir, file_path, status_path))
  {
    log.error << "Bad URL: " << url << endl;
    return false;
  }

  log.detail << "URL maps to " << file_path << endl;

  XML::Configuration status_cfg(status_path.str(), log.error);

  if (status_path.exists())
    status_cfg.read("status");  // Read it
  else
    status_cfg.replace_root("status"); // Create root

  // Set URL
  status_cfg.ensure_path("source");
  status_cfg.set_value("source/@url", url.str());

  // Get last check time
  if (check_for_updates)
  {
    Time::Duration check_interval(status_cfg["update/check/@interval"]);
    if (!check_interval)
    {
      // Don't do any updates
      check_for_updates = false;
    }
    else
    {
      // Check if within the interval
      Time::Stamp last_check(status_cfg["update/check/@time"]);
      if (!!last_check)
	log.detail << "Last checked at " << last_check.iso() << endl;
      else
	log.detail << "This is the first check\n";

      // Does it need checking again?
      if (Time::Stamp::now() - last_check < check_interval)
      {
	log.detail << "Doesn't need checking again until " 
		   << (last_check+check_interval).iso() << endl;
	check_for_updates = false;
      }
    }
  }

  // If no update check required, and it exists, that's enough
  if (!check_for_updates && file_path.exists())
  {
    path_p = file_path;
    return true;
  }

  URL actual_url = url;

  for(int i=0; i<MAX_REDIRECTS; i++)
  {
    log.detail << "Fetch required from " << actual_url << endl;

    // Need to fetch it
    Web::HTTPClient client(actual_url, ssl_ctx, user_agent, 5, 5);
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
	string err = file_path.write_all(response.body);
	if (!err.empty())
	{
	  log.error << "Can't write cache file '" << file_path 
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
	status_cfg.ensure_path("update/check");
	status_cfg.set_value("update/check/@time", Time::Stamp::now().iso()); 

	status_cfg.write();

	path_p = file_path;
	return true;
      }

      case 301: case 302:  // Moved
	actual_url = Web::URL(response.headers.get("location"));
	log.detail << "Redirect to " << actual_url << endl;
	break;  // Loops to retry fetch

      case 304:  // Not modified
	// Update last check time only
	status_cfg.ensure_path("update/check");
	status_cfg.set_value("update/check/@time", Time::Stamp::now().iso()); 
	status_cfg.write();
	path_p = file_path;
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
bool Cache::fetch(const URL& url, string& contents_p, bool check_for_updates)
{
  File::Path path;
  return fetch(url, path, check_for_updates) && path.read_all(contents_p);
}

//--------------------------------------------------------------------------
// Set the update check interval for a given URL
// interval is in Time::Duration constructor format
// (URL must already have been fetched)
bool Cache::set_update_interval(const URL& url, const string& interval)
{
  Log::Streams log;
  log.detail << "Setting update interval for " << url << " to " 
	     << interval << endl;

  File::Directory domain_dir;
  File::Path file_path, status_path;
  if (!get_paths(url, domain_dir, file_path, status_path)) return false;

  XML::Configuration status_cfg(status_path.str(), log.error);
  if (!status_cfg.read("status")) return false;

  // Set the interval
  status_cfg.ensure_path("update/check");
  status_cfg.set_value("update/check/@interval", interval);
  return status_cfg.write();
}

//--------------------------------------------------------------------------
// Get the cache paths for a given URL
bool Cache::get_paths(const URL& url,
		      File::Directory& domain_dir_p,
		      File::Path& file_path_p,
		      File::Path& status_path_p)
{
  XML::Element url_xml;
  if (!url.split(url_xml)) return false;
  XML::XPathProcessor xpath(url_xml);

  // Construct domain directory from URL domain
  domain_dir_p = File::Directory(directory, xpath["host"]);

  // Get filename base from the MD5 of the whole URL
  Misc::MD5 md5;
  string base = md5.sum(url.get_text());

  // Get the extension of the path
  string upath = xpath["path"];
  string::size_type p = upath.rfind('.');
  if (p != string::npos) 
  {
    string ext(upath, p);

    // This must not contain any slashes, otherwise the dot was in some
    // earlier part of the filename
    if (ext.find('/') == string::npos) base+=ext;
  }

  // Construct the filename
  file_path_p = File::Path(domain_dir_p, base);

  // Construct the status filename
  status_path_p = File::Path(domain_dir_p, STATUS_PREFIX+base+STATUS_SUFFIX);

  return true;
}

//--------------------------------------------------------------------------
// Update the cache in background
// Runs a single time through the entire cache, checking for updates on files
// with update intervals set
void Cache::update()
{
  Log::Streams log;

  // Scan all domain directories
  list<File::Path> dirs;
  if (!directory.inspect(dirs)) return;

  for(list<File::Path>::iterator p = dirs.begin(); p!=dirs.end(); ++p)
  {
    File::Directory dir(*p);
    log.detail << "Updating cache directory " << dir << endl;

    // Find all status files
    list<File::Path> files;
    if (!dir.inspect(files, string(STATUS_PREFIX)+"*"+STATUS_SUFFIX, true))
      continue;

    for(list<File::Path>::iterator q = files.begin(); q!=files.end(); ++q)
    {
      File::Path& file = *q;
      log.detail << "Checking file " << file << endl;

      XML::Configuration status_cfg(file.str(), log.error);
      if (!status_cfg.read("status")) continue;

      string url = status_cfg["source/@url"];
      log.detail << "Source URL " << url << endl;

      string interval = status_cfg["update/check/@interval"];
      if (!interval.empty())
      {
	log.detail << "Update interval is " << interval << endl;

	// Try to fetch it, with update check
	File::Path path;
	fetch(Web::URL(url), path, true);
      }
    }
  }
}



}} // namespaces



