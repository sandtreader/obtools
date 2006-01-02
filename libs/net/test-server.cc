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
  TestServer(): ObTools::Net::TCPServer(5000) {}

  void process(ObTools::Net::TCPSocket& s, 
	       ObTools::Net::EndPoint client);
};

void TestServer::process(ObTools::Net::TCPSocket& s, 
			 ObTools::Net::EndPoint client)
{
  cerr << "Got connection from " << client << endl;

  try
  {
    // Just output what we get
    string buf;
    while (s >> buf) cout << buf;

    cerr << "Connection from " << client << " ended\n";
  }
  catch (ObTools::Net::SocketError se)
  {
    cerr << se << endl;
  }
} 


//--------------------------------------------------------------------------
// Main

int main()
{
#if defined(__WIN32__)
  winsock_initialise();
#endif

  TestServer server;
  server.run();
  return 0;
}




