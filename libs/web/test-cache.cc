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
using namespace std;
using namespace ObTools;

#define CACHE_DIR "/tmp/ot-web-cache"

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cout << "Usage: " << argv[0] << " <url>\n";
    return 2;
  }

#ifdef __WIN32__
  winsock_initialise();
#endif

  Log::StreamChannel chan_out(cerr);
  Log::logger.connect(chan_out);
  Log::Streams log;

  Web::URL url(argv[1]);

  // Create directory
  File::Directory dir(CACHE_DIR);
  dir.ensure();

  // Create cache
  Web::Cache cache(dir, Time::Duration("1 min"));

  // Fetch URL
  File::Path path;
  if (cache.fetch(url, path))
  {
    string contents;
    if (!path.read_all(contents))
    {
      log.error << "Can't read back " << path << ": " << strerror(errno) 
		<< endl;
      return 2;
    }

    log.summary << "Read data: " << contents.size() << " bytes\n";
    return 0;
  }
  else
    return 2;
}




