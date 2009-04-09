//==========================================================================
// ObTools::Web:: test-http-client.cc
//
// Test harness for HTTP message functions
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-web.h"
#include "ot-log.h"
using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cout << "Usage: " << argv[0] << " [-p] <url>\n";
    cout << "       -p indicates POST using input from stdin\n";
    return 2;
  }

#ifdef __WIN32__
  winsock_initialise();
#endif

  Log::StreamChannel chan_out(cerr);
  Log::logger.connect(chan_out);
  Log::Streams log;

  int i=1;
  string in;
  bool post = false;
  if (string(argv[i]) == "-p")
  {
    post = true;
    i++;

    // Read stdin
    while (cin) in += cin.get();
  }

  Web::URL url(argv[i]);
  log.summary << "Test HTTP client " << (post?"posting":"fetching") 
	      << " " << url << endl;

  // Create SSL Context
  SSL::Context ctx;


  Web::HTTPClient client(url, &ctx, "ObTools Test HTTP client", 5, 5);
  string body;
  int result = post?client.post(url, in, body):client.get(url, body);
  if (result == 200)
  {
    cout << body << endl;
    cout << "We connected from " << client.get_last_local_address() << endl;
    return 0;
  }
  else
  {
    log.error << "Failed: code " << result << " - " << body << endl;
    return 1;
  }
}




