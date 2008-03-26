//==========================================================================
// ObTools::Net: test-http-server.cc
//
// Test HTTP server - just receives request and sends back junk
//
// Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-web.h"
#include <sstream>
#include <fstream>

using namespace std;
using namespace ObTools;

#define SERVER_PORT 33333
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
int main(int argc, char **argv)
{
#ifdef __WIN32__
  winsock_initialise();
#endif

  Log::StreamChannel chan_out(cout);
  Log::logger.connect(chan_out);
  Log::Streams log;

  int port = SERVER_PORT;
  if (argc > 1) port = atoi(argv[1]);
  log.summary << "Test HTTP server running on port " << port << endl;

  // Use SSL
  SSL::Context *ctx_p = 0;
  SSL::Context ctx;

  if (argc > 3)
  {
    // Read certificate
    string certfile(argv[2]);
    ifstream cf(certfile.c_str());
    if (!cf)
    {
      cerr << "Can't read certificate file: " << certfile << endl;
      return 2;
    }

    // Read certificate
    Crypto::Certificate cert;
    cf >> cert;

    if (!cert)
    {
      cerr << "Bad certificate file: " << certfile << endl;
      return 2;
    }

    cout << "Certificate read for CN " << cert.get_cn() << endl;

    // Add certificate to context
    ctx.use_certificate(cert);

    // Read private key
    string pkfile(argv[3]);
    ifstream pkf(pkfile.c_str());
    if (!pkf)
    {
      cerr << "Can't read private key file: " << pkfile << endl;
      return 2;
    }

    // Read private key
    Crypto::RSAKey rsa(true);
    pkf >> rsa;

    if (!rsa.valid)
    {
      cerr << "Bad key file: " << pkfile << endl;
      return 2;
    }

    // Add private key to context
    ctx.use_private_key(rsa);

    // Use it
    ctx_p = &ctx;
  }

  Web::SimpleHTTPServer server(ctx_p, port, SERVER_VERSION);
  server.add(new TestURLHandler());
  server.add(new DefaultURLHandler());

  server.run();
  return 0;
}




