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
    ObTools::Web::HTTPMessage msg;

    if (msg.read(ss))
    {
      cout << msg.version << " request: " << msg.method << " for " 
	   << msg.url << endl;
      cout << msg.headers.xml;
      if (msg.body.size()) cout << "Body:\n" << msg.body << endl;

      // Send back a nice response
      ObTools::Web::HTTPMessage response(200, "OK");
      response.headers.put("server", "ObTools Web test server");
      response.body = "<TITLE>That worked</TITLE><P>Thanks!</P>\n";

      if (!response.write(ss))
      {
	cerr << "HTTP response generation failed\n";
      }
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




