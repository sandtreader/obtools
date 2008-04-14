//==========================================================================
// ObTools::SOAP: test-url-handler.cc
//
// Test URL handler - just receives request and sends back junk
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-soap.h"
#include <sstream>

using namespace std;
using namespace ObTools;

#define SERVER_PORT 5080
#define SERVER_VERSION "ObTools Test HTTP Server"

//--------------------------------------------------------------------------
// Handler for /test*
class TestURLHandler: public SOAP::URLHandler
{
  // Implementation of message handler
  bool handle_message(SOAP::Message& request, SOAP::Message& response,
		      Web::HTTPMessage&, Web::HTTPMessage&,
		      SSL::ClientDetails& client)
  {
    Log::Streams log;
    log.summary << "SOAP request from " << client << endl;
    log.detail << request << endl;

    // Create a Fault to send back
    SOAP::MustUnderstandFault fault;
    response.take(fault);

    return true;
  }

public:
  TestURLHandler(): SOAP::URLHandler("/test*") {}
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

  log.summary << "Test SOAP server running on port " << SERVER_PORT << endl;

  Web::SimpleHTTPServer server(SERVER_PORT, SERVER_VERSION);
  server.add(new TestURLHandler());

  server.run();
  return 0;
}




