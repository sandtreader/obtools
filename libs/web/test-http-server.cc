//==========================================================================
// ObTools::Net: test-http-server.cc
//
// Test HTTP server - just receives request and sends back junk
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-web.h"
#include <sstream>

using namespace std;
using namespace ObTools;

#define SERVER_PORT 5080
#define SERVER_VERSION "ObTools Test HTTP Server"

//--------------------------------------------------------------------------
// Handler for /test*
class TestURLHandler: public Web::URLHandler
{
public:
  TestURLHandler(): URLHandler("/test*") {}

  bool handle_request(Web::HTTPMessage& request, Web::HTTPMessage& response,
		      Net::EndPoint client)
  {
    ostringstream oss;
    oss << "<TITLE>" << SERVER_VERSION << "</TITLE>\n";
    oss << "<H1>" << SERVER_VERSION << "</H1>\n";
    oss << "<P>" << request.method << " request from " << client << endl;
    oss << "<P>URL: " << request.url << endl;
    oss << "<P>Body: " << request.body << endl;
    response.body = oss.str();
    
    return true;
  }
};

//--------------------------------------------------------------------------
// Default handler
class DefaultURLHandler: public Web::URLHandler
{
public:
  DefaultURLHandler(): URLHandler("*") {}

  bool handle_request(Web::HTTPMessage& /*request*/, 
		      Web::HTTPMessage& response,
		      Net::EndPoint /*client*/)
  {
    ostringstream oss;
    oss << "<TITLE>" << SERVER_VERSION << "</TITLE>\n";
    oss << "<H1>" << SERVER_VERSION << "</H1>\n";
    oss << "<P>Nothing registered for this url\n";
    oss << "<P>Please try <A HREF='/test/foo'>/test</A>\n";
    response.body = oss.str();
    
    return true;
  }
};

//--------------------------------------------------------------------------
// Main
int main()
{
#ifdef __WIN32__
  winsock_initialise();
#endif

  Log::StreamChannel chan_out(cout);
  Log::logger.connect(chan_out);
  Log::Streams log;

  log.summary << "Test HTTP server running on port " << SERVER_PORT << endl;

  Web::SimpleHTTPServer server(SERVER_PORT, SERVER_VERSION);
  server.add(new TestURLHandler());
  server.add(new DefaultURLHandler());

  server.run();
  return 0;
}




