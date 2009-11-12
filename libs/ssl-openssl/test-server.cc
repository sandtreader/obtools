//==========================================================================
// ObTools::SSL: test-server.cc
//
// Test harness for SSL server functions
//
// Copyright (c) 2008 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-ssl-openssl.h"
#include "ot-log.h"
#include <fstream>
#include <signal.h>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Test server class
class TestServer: public SSL::TCPServer
{
public:
  TestServer(SSL::Context *ctx, int port): 
    SSL::TCPServer(ctx, port) {}

  void process(Net::TCPSocket& s, 
	       Net::EndPoint client);
};

void TestServer::process(Net::TCPSocket& s, 
			 Net::EndPoint client)
{
  cerr << "Got connection from " << client 
       << " (" << s.get_mac(client.host) << ")" << endl;

  try
  {
    // Just output and reflect what we get
    string buf;
    while (s >> buf)
    {
      cout << buf;
      s << "<< " << buf << "\n";
    }

    cerr << "Connection from " << client << " ended\n";
  }
  catch (Net::SocketError se)
  {
    cerr << se << endl;
  }
} 


//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
#if defined(__WIN32__)
  winsock_initialise();
#else
  // Force ignore for SIGPIPE
  signal(SIGPIPE, SIG_IGN);
#endif

  if (argc < 2)
  {
    cout << "Usage:\n";
    cout << " test-server <port> [<cert file> <private key file>]\n";
    return 0;
  }

  Log::StreamChannel chan_out(cerr);
  Log::logger.connect(chan_out);

  Crypto::Library crypto;

  int port = 11111;
  port = atoi(argv[1]);

  if (argc > 2)
  {
    SSL_OpenSSL::Context ctx;

    // Read certificate
    string certfile(argv[2]);
    ifstream cf(certfile.c_str());
    if (!cf)
    {
      cerr << "Can't read certificate file: " << certfile << endl;
      return 2;
    }

    // Read certificate
    Crypto::Certificate cert;
    cf >> cert;

    if (!cert)
    {
      cerr << "Bad certificate file: " << certfile << endl;
      return 2;
    }

    cout << "Certificate read for CN " << cert.get_cn() << endl;

    // Add certificate to context
    ctx.use_certificate(cert);

    // Read private key
    string pkfile(argv[3]);
    ifstream pkf(pkfile.c_str());
    if (!pkf)
    {
      cerr << "Can't read private key file: " << pkfile << endl;
      return 2;
    }

    // Read private key
    Crypto::RSAKey rsa(true);
    pkf >> rsa;

    if (!rsa.valid)
    {
      cerr << "Bad key file: " << pkfile << endl;
      return 2;
    }

    // Add private key to context
    ctx.use_private_key(rsa);

    cout << "Starting SSL server on port " << port << endl;
    TestServer server(&ctx, port);

    server.run();
  }
  else
  {
    // Just run normally
    cout << "Starting plain server on port " << port << endl;
    TestServer server(0, port);

    server.run();
  }

  return 0;
}




