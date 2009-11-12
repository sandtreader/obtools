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

//--------------------------------------------------------------------------
// Test server class
class TestServer: public ObTools::Net::TCPServer
{
public:
  TestServer(int port): ObTools::Net::TCPServer(port) {}

  void process(ObTools::Net::TCPSocket& s, 
	       ObTools::Net::EndPoint client);
};

void TestServer::process(ObTools::Net::TCPSocket& s, 
			 ObTools::Net::EndPoint client)
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
  catch (ObTools::Net::SocketError se)
  {
    cerr << se << endl;
  }
} 


//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  int port = 11111;
  if (argc > 1) port = atoi(argv[1]);

#if defined(__WIN32__)
  winsock_initialise();
#endif

  cout << "Starting server on port " << port << endl;
  TestServer server(port);

  server.run();

  return 0;
}




