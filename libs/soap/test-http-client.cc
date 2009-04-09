//==========================================================================
// ObTools::SOAP:: test-http-client.cc
//
// Test harness for SOAP HTTP functions
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-soap.h"
#include "ot-log.h"
using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    cout << "Usage: " << argv[0] << " <url> <soap action>\n";
    cout << "  Accepts SOAP input from stdin\n";
    return 2;
  }

#ifdef __WIN32__
  winsock_initialise();
#endif

  Log::StreamChannel chan_out(cerr);
  Log::logger.connect(chan_out);
  Log::Streams log;

  // Read stdin
  string in;
  while (cin) in += cin.get();

  Web::URL url(argv[1]);
  string soap_action(argv[2]);

  log.summary << "Test SOAP client to " << url << "(" << soap_action << ")\n";
  SOAP::HTTPClient client(url, "ObTools Test SOAP client", 5);

  // Create SOAP message
  SOAP::Parser parser(log.error);
  SOAP::Message request(in, parser);

  if (!request)
  {
    log.error << "Invalid SOAP\n";
    return 2;
  }

  SOAP::Message response;
  int result = client.post(url, soap_action, request, response);
  if (result == 200)
  {
    cout << response << endl;
    return 0;
  }
  else
  {
    log.error << "Failed: code " << result << ":\n" << response << endl;
    return 1;
  }
}




