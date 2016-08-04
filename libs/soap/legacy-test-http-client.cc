//==========================================================================
// ObTools::SOAP:: test-http-client.cc
//
// Test harness for SOAP HTTP functions
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-soap.h"
#include "ot-log.h"
using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cout << "Usage: " << argv[0] << " <url> [<soap action>]\n";
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
  while (cin)
  {
    char c = cin.get();
    if (c > 0) in+=c;
  }

  // If it doesn't look like SOAP already, wrap it up
  if (in.find("env:Envelope") == string::npos)
  {
    in = "<?xml version='1.0' encoding='UTF-8'?>\n"
         "<env:Envelope xmlns:env='http://schemas.xmlsoap.org/soap/envelope/'>\n"
         "  <env:Body>\n"
         +in+
         "  </env:Body>\n"
         "</env:Envelope>\n";
  }

  Web::URL url(argv[1]);
  string soap_action;
  if (argc > 2) soap_action = argv[2];

  log.summary << "Test SOAP client to " << url;
  if (!soap_action.empty()) log.summary << " (" << soap_action << ")";
  log.summary << endl;

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




