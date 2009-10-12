//==========================================================================
// ObTools::Web:: test-cache.cc
//
// Test harness for HTTP cache functions
//
// Copyright (c) 2009 ObTools Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-web.h"
#include "ot-log.h"
#include "errno.h"

using namespace std;
using namespace ObTools;

#define CACHE_DIR "/tmp/ot-web-cache"

// Usage:
//   test-cache [url [update interval]]

// If no URL given, does an update

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
#ifdef __WIN32__
  winsock_initialise();
#endif

  Log::StreamChannel chan_out(cerr);
  Log::logger.connect(chan_out);
  Log::Streams log;

  // Create directory
  File::Directory dir(CACHE_DIR);
  dir.ensure();

  // Create cache
  Web::Cache cache(dir);

  if (argc > 1)
  {
    Web::URL url(argv[1]);

    // Fetch URL
    File::Path path;
    if (cache.fetch(url, path, true))
    {
      string contents;
      if (!path.read_all(contents))
      {
	log.error << "Can't read back " << path << ": " << strerror(errno) 
		  << endl;
	return 2;
      }
      
      log.summary << "Read data: " << contents.size() << " bytes\n";

      // If second argument set, use it as interval
      if (argc>2) cache.set_update_interval(url, argv[2]);

      return 0;
    }
    else
      return 2;
  }
  else
  {
    // Just update it
    cache.update();
  }
}




