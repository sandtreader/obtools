//==========================================================================
// ObTools::Net: test-client.cc
//
// Test harness for network library client functions
// Simulates a dumb Web client
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-net.h"
#include <stdlib.h>

using namespace std;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cerr << "Give a hostname and optional port\n" << endl;
    return 2;
  }

  char *host = argv[1];
  int port = 80;
  if (argc > 2) port = atoi(argv[2]);

#if defined(PLATFORM_WINDOWS)
  winsock_initialise();
#endif

  ObTools::Net::IPAddress addr(host);
  if (!addr)
  {
    cerr << "Can't resolve host: " << host << endl;
    return 1;
  }

  cout << "Host: " << addr << " (" << addr.get_hostname() << ")" << endl;

  ObTools::Net::EndPoint ep(addr, port);
  ObTools::Net::TCPClient client(ep);

  if (!client)
  {
    cerr << "Can't connect to host\n";
    return 1;
  }

  try
  {
    // Write HTTP request
    client << "GET / HTTP/1.0\r\n\r\n";

    // Read the result back
    string s;
    while (client >> s) cout << s;
  }
  catch (const ObTools::Net::SocketError& se)
  {
    cerr << se << endl;
    return 1;
  }

  return 0;
}




