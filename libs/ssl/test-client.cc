//==========================================================================
// ObTools::SSL: test-client.cc
//
// Test harness for SSL library client functions
// Simulates a dumb Web client
//
// Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-ssl.h"
#include "ot-log.h"

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  Log::StreamChannel chan_out(cerr);
  Log::logger.connect(chan_out);

  if (argc < 2)
  {
    cerr << "Give a hostname and optional port\n" << endl;
    return 2;
  }

  char *host = argv[1];
  int port = 80;
  if (argc > 2) port = atoi(argv[2]);

#ifdef __WIN32__
  winsock_initialise();
#endif

  Net::IPAddress addr(host);
  if (!addr)
  {
    cerr << "Can't resolve host: " << host << endl;
    return 1;
  }

  cout << "Host: " << addr << " (" << addr.get_hostname() << ")" << endl;

  // Create SSL Context
  SSL::Context ctx;

  Net::EndPoint ep(addr, port);
  SSL::TCPClient client(&ctx, ep);

  if (!client)
  {
    cerr << "Can't connect to host\n";
    return 1;
  }

  // Get server's CN
  cout << "Server's CN is " << client.get_peer_certificate().get_cn() << endl;

  try
  {
    // Write HTTP request
    client << "GET / HTTP/1.0\r\n\r\n";

    // Read the result back
    string s;
    while (client >> s) cout << s;
  }
  catch (Net::SocketError se)
  {
    cerr << se << endl;
    return 1;
  }
   
  return 0;  
}




