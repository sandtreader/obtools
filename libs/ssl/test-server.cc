//==========================================================================
// ObTools::SSL: test-server.cc
//
// Test harness for SSL server functions
//
// Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-ssl.h"
#include "ot-log.h"
#include <fstream>

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
  if (argc < 4)
  {
    cout << "Usage:\n";
    cout << " test-server <port> <cert file> <private key file>\n";
    return 0;
  }

  Log::StreamChannel chan_out(cerr);
  Log::logger.connect(chan_out);

  int port = 11111;
  port = atoi(argv[1]);

  SSL::Context ctx;

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

#if defined(__WIN32__)
  winsock_initialise();
#endif

  cout << "Starting server on port " << port << endl;
  TestServer server(&ctx, port);

  server.run();

  return 0;
}




