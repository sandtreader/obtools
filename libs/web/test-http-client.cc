//==========================================================================
// ObTools::Web:: test-http-client.cc
//
// Test harness for HTTP message functions
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-web.h"
#include "ot-ssl-openssl.h"
#include "ot-log.h"

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cout << "Usage: " << argv[0] << " [-p] [-1] <url> [<url>]*\n";
    cout << "       -p indicates POST using input from stdin\n";
    cout << "       -1 Use HTTP/1.1\n";
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
  bool http_1_1 = false;
  if (string(argv[i]) == "-p")
  {
    post = true;
    i++;

    // Read stdin
    while (cin)
    {
      int c = cin.get();
      if (c>=0) in += c;
    }
  }

  if (string(argv[i]) == "-1")
  {
    http_1_1 = true;
    i++;
  }

  // Create SSL Context
  SSL_OpenSSL::Context ctx;
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




