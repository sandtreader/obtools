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
    ObTools::XML::Element root("HTTP");
    ObTools::Web::HTTPMessageParser hmp(root, ss);

    if (hmp.parse())
    {
      cout << root;

      // Send back a nice response
      string body = "<TITLE>That worked</TITLE><P>Thanks!</P>\n";
      ObTools::XML::Element response("HTTP");
      response.set_attr("code", "200");
      response.set_attr("version", "HTTP/1.0");
      response.add("reason", "OK");
      ObTools::XML::Element& headers = response.add("headers");
      headers.add("server", "ObTools Web test server");
      response.add("body", body);

      ObTools::Web::HTTPMessageGenerator hmg(response, ss);
      if (!hmg.generate())
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




