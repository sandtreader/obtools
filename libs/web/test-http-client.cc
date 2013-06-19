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

#define PROGRESSIVE_BUF_SIZE 65536

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cout << "Usage: " << argv[0] << " [options] <url> [<url>]*\n\n";
    cout << "Options:\n";
    cout << "       -p   POST using input from stdin\n";
    cout << "       -P   Progressive download\n";
    cout << "       -1   Use HTTP/1.1\n";
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
  bool progressive = false;
  bool http_1_1 = false;

  for(;i<argc;i++)
  {
    string arg = argv[i];
    if (arg[0] != '-') break;

    if (arg == "-p") post = true;
    else if (arg == "-P") progressive = true;
    else if (arg == "-1") http_1_1 = true;
    else 
    {
      cerr << "Unknown option " << arg << endl;
      return 2;
    }
  }

  // Read cin for post
  if (post)
  {
    // Read stdin
    while (cin)
    {
      int c = cin.get();
      if (c>=0) in += c;
    }
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
      if (progressive) client->enable_progressive();
    }

    // Set persistent close on last URL
    if (http_1_1 && i==argc-1) client->close_persistence();

    string body;
    int result = post?client->post(url, in, body):client->get(url, body);
    if (result == 200)
    {
      log.detail << "We connected from " << client->get_last_local_address() 
		 << endl;

      if (progressive)
      {
	// Read progressive data in small chunks
	unsigned char buf[PROGRESSIVE_BUF_SIZE];
	unsigned int n;
	uint64_t total = 0;
	while ((n = client->read(buf, PROGRESSIVE_BUF_SIZE)) != 0)
	{
	  log.detail << "Read buffer " << n << endl;
	  cout.write(reinterpret_cast<char *>(buf), n);
	  total += n;
	}

	log.detail << "Read total of " << total << " bytes\n";
      }
      else cout << body << endl;
    }
    else
    {
      log.error << "Failed: code " << result << " - " << body << endl;
      delete client;
      return 1;
    }
  }

  if (client) delete client;
  return 0;
}




