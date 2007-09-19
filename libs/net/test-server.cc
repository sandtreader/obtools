//==========================================================================
// ObTools::Net: test-server.cc
//
// Test harness for TCP server functions
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-net.h"
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
  int iport = 0;
  if (argc > 1) port = atoi(argv[1]);
  if (argc > 2) iport = atoi(argv[2]);

#if defined(__WIN32__)
  winsock_initialise();
#endif

  cout << "Starting server on port " << port << endl;
  TestServer server(port);

  if (iport)
  {
    ObTools::Net::EndPoint ep(ObTools::Net::IPAddress("127.0.0.1"), iport);
    ObTools::Net::TCPSocket client(server.initiate(ep, 5));

    if (!client)
    {
      cerr << "Can't initiate peer connection to " << ep << endl;
      return 2;
    }

    client << "Hello world\n";
    ObTools::MT::Thread::sleep(1);
  }
  else
  {
    server.run();
  }
  return 0;
}




