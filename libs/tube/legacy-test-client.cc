//==========================================================================
// ObTools::Tube: test-client.cc
//
// Test harness for tube client
//
// Copyright (c) 2007 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-tube.h"
#include "ot-file.h"
#include "ot-ssl-openssl.h"
#include <stdlib.h>
#include <signal.h>

#if defined(__WIN32__)
#include <windows.h>
#endif

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main
#pragma GCC diagnostic ignored "-Wold-style-cast"
const sighandler_t sig_ign(SIG_IGN);
#pragma GCC diagnostic pop
int main(int argc, char **argv)
{
  if (argc < 3)
  {
    cout << "Usage: " << argv[0] << " [options] hostname port\n\n";
    cout << "Options:\n";
    cout << "   -n <num>           Repeat N times\n";
    cout << "   -ssl               Use SSL context\n";
    cout << "   -cert <cert> <key> Use given certificate and key files\n";
    cout << "   -pass <phrase>     Pass phrase for key\n";
    cout << "   -tag <tag>         Use given message tag\n";
    cout << "   -r                 Request result\n";
    cout << "   -stdin             Read data from stdin\n";
    return 2;
  }

  Tube::tag_t tag = 0x12345678;
  int n = 1;
  bool ssl = false;
  string ssl_cert;
  string ssl_key;
  string ssl_pass;
  bool result = false;
  string data = "Hello, world!\n";

  int i;
  for(i=1; i<argc-2; i++)
  {
    string arg = argv[i];
    if (arg == "-n" && i<argc-3)
      n = atoi(argv[++i]);
    else if (arg == "-ssl")
      ssl = true;
    else if (arg == "-cert" && i<argc-4)
    {
      ssl_cert = argv[++i];
      ssl_key  = argv[++i];
    }
    else if (arg == "-pass" && i<argc-3)
    {
      ssl_pass = argv[++i];
    }
    else if (arg == "-tag" && i<argc-3)
    {
      char *s = argv[++i];
      tag = (static_cast<uint32_t>(s[0])<<24)
          | (static_cast<uint32_t>(s[1])<<16)
          | (static_cast<uint32_t>(s[2])<< 8)
          | (static_cast<uint32_t>(s[3]));
    }
    else if (arg == "-r")
      result = true;
    else if (arg == "-stdin")
    {
      data.clear();

      // Read stdin
      while (cin)
      {
	int c = cin.get();
	if (c>=0) data += c;
      }
    }
    else
    {
      cerr << "Unrecognised option " << arg << endl;
      return 2;
    }
  }

  char *host = argv[i++];
  int port = atoi(argv[i]);

  // Set up logging
  Log::StreamChannel   chan_out(cout);
  Log::TimestampFilter tsfilter("%H:%M:%S %a %d %b %Y: ", chan_out);
  Log::LevelFilter     level_out(Log::LEVEL_DUMP, tsfilter);
  Log::logger.connect(level_out);
  Log::Streams log;

#ifdef __WIN32__
  winsock_initialise();
#else
  // Ignore SIGPIPE (not quite why we're getting them!)
  signal(SIGPIPE, sig_ign);
#endif

  // Resolve name
  Net::IPAddress addr(host);
  if (!addr)
  {
    log.error << "Can't resolve host: " << host << endl;
    return 1;
  }

  log.summary << "Host: " << addr 
	      << " (" << addr.get_hostname() << ")" << endl;

  // Get SSL context
  SSL_OpenSSL::Context ssl_ctx;

  if (ssl)
  {
    File::Path key_file(ssl_key);
    string key_pem;
    if (!key_file.read_all(key_pem))
    {
      log.error << "Can't read key file " << key_file 
		<< ": " << key_pem << endl;
      return 4;
    }

    // Get private key
    Crypto::RSAKey rsa(key_pem, true, ssl_pass);
    if (!rsa.valid)
    {
      log.error << "Invalid RSA private key or pass phrase - giving up\n";
      return 4;
    }

    ssl_ctx.use_private_key(rsa);

    File::Path cert_file(ssl_cert);
    string cert_pem;
    if (!cert_file.read_all(cert_pem))
    {
      log.error << "Can't read certificate file " << cert_file 
		<< ": " << cert_pem << endl;
      return 4;
    }

    if (!ssl_ctx.use_certificate(cert_pem))
    {
      log.error << "Bad certificate file " << cert_file << endl;
      return 4;
    }
  }

  // Start client
  Net::EndPoint server(addr, port);

  // Use different clients depending whether we need a result or not
  if (result)
  {
    Tube::AutoSyncClient client(server, ssl?&ssl_ctx:0);

    // Loop for a while sending and receiving
    for(int i=0; i<n ; i++)
    {
      if (i) this_thread::sleep_for(chrono::seconds{1});
      Tube::Message msg(tag, data);
      Tube::Message response;

      if (client.request(msg, response))
	cout << response.data;
    }

    log.summary << "Shutting down\n";
    client.shutdown();
  }
  else
  {
    Tube::Client client(server, ssl?&ssl_ctx:0);
    client.start();

    // Loop for a while just sending 
    for(int i=0; i<n; i++)
    {
      if (i) this_thread::sleep_for(chrono::seconds{1});
      Tube::Message msg(tag, data);

      client.send(msg);
    }

    log.summary << "Shutting down\n";
    client.shutdown();
  }

  log.summary << "Done\n";

  return 0;  
}




