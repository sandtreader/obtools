//==========================================================================
// ObTools::Net: test-server.cc
//
// Test harness for TCP server functions
// Simulates a basic Web server
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
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
	       ObTools::Net::IPAddress client_address,
	       int client_port);
};

void TestServer::process(ObTools::Net::TCPSocket& s, 
			 ObTools::Net::IPAddress client_address,
			 int client_port)
{
  cerr << "Got connection from " << client_address << ":" << client_port 
       << endl;

  try
  {
    // Just reflect
    string buf;
    while (s >> buf) s << buf;

    cerr << "Connection from " << client_address << ":" << client_port 
	 << " ended\n";
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
  TestServer server;
  server.run();
  return 0;
}




