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
    cout << "Usage: " << argv[0] << " [-p] <url> [<url>]*\n";
    cout << "       -p indicates POST using input from stdin\n";
    cout << "If more than one URL is given, HTTP/1.1 persistent connections are used\n";
    cout << "(must be to same host)\n";
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

  // Create SSL Context
  SSL::Context ctx;
  
  bool http_1_1 = (argc > i+1);  // Multiple URLs?
  Web::HTTPClient *client = 0;

  for(;i<argc;i++)
  {
    Web::URL url(argv[i]);
    log.summary << "Test HTTP client " << (post?"posting":"fetching") 
		<< " " << url << endl;

    // Create client on first URL
    if (!client)
    {
      client = new Web::HTTPClient(url, &ctx, 
				   "ObTools Test HTTP client", 5, 5);
      if (http_1_1) client->enable_persistence();
    }

    // Set persistent close on last URL
    if (http_1_1 && i==argc-1) client->close_persistence();

    string body;
    int result = post?client->post(url, in, body):client->get(url, body);
    if (result == 200)
    {
      log.detail << "We connected from " << client->get_last_local_address() 
		 << endl;

      cout << body << endl;
    }
    else
    {
      log.error << "Failed: code " << result << " - " << body << endl;
      return 1;
    }
  }

  if (client) delete client;
  return 0;
}




