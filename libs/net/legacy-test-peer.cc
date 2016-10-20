//==========================================================================
// ObTools::Net: test-server.cc
//
// Test harness for TCP server functions
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-net.h"
#include <stdlib.h>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Test server class
class TestServer: public Net::TCPServer
{
public:
  TestServer(int port): Net::TCPServer(port) {}
  TestServer(Net::EndPoint ep): Net::TCPServer(ep) {}

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
  catch (const Net::SocketError& se)
  {
    cerr << se << endl;
  }
}


//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    cerr << "Usage:\n  " << argv[0] << " <peer host> <port> [<local addr>]\n";
    return 2;
  }

  string peer = argv[1];
  int port = atoi(argv[2]);
  string local = "0.0.0.0";
  if (argc > 3) local = argv[3];

#if defined(__WIN32__)
  winsock_initialise();
#endif

  Net::IPAddress local_ip(local);
  Net::EndPoint local_ep(local_ip, port);
  Net::IPAddress peer_ip(peer);
  Net::EndPoint peer_ep(peer_ip, port);


  // !!! Start client to see if we can bind _before_ listen
  Net::TCPSocket client;
  client.enable_reuse();
  client.bind(local_ep);

  cout << "Starting server on port " << port << endl;
  TestServer server(local_ep);
  Net::TCPServerThread server_thread(server);

    this_thread::sleep_for(chrono::seconds{1});

  for(;;)
  {
    this_thread::sleep_for(chrono::seconds{1});
#if 0
    cout << "Starting outgoing connection to " << peer_ep << endl;

    Net::TCPSocket client(server.initiate(peer_ep, 5));

    if (!client)
    {
      cerr << "Can't initiate peer connection to " << peer_ep << endl;
      continue;
    }

    client << "Hello world\n";
#endif
  }

  return 0;
}




