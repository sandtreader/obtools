//==========================================================================
// ObTools::Net: test-http-server.cc
//
// Test HTTP server - just receives request and sends back junk
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-web.h"
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
    ObTools::Net::TCPStream ss(s);
    ObTools::XML::Element root;
    ObTools::Web::HTTPMessageParser hmp(root, ss);

    if (hmp.parse())
    {
      cout << root;
      ss << "200 OK\r\n";
    }
    else
    {
      cerr << "HTTP Parse failed\n";
    }
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
  TestServer server;
  server.run();
  return 0;
}




