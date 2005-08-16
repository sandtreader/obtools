// =======================================================================
// ObTools::XMLMesh: xmlmesh-send.cc
//
// Command-line interface to XMLMesh messaging
//
// Accepts message to send on stdin, and may also output response on stdout
// - see usage() below
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
// ======================================================================= 

#include "ot-xmlmesh-client-otmp.h"
#include "ot-log.h"

#if !defined(_SINGLE)
#error Do NOT build multithreaded - no guarantee that send() will complete!
#endif

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Usage
void usage(char *pname)
{
  cout << "ObTools XMLMesh command line message interface\n\n";
  cout << "Usage:\n";
  cout << "  " << pname << " [options] <subject>\n\n";
  cout << "Reads message from stdin and sends it with the given subject\n";
  cout << "May output response to stdout if requested\n";
  cout << "Result code 0 for success, 1 for message failure, 2 for fatal error\n\n";
  cout << "Options:\n";
  cout << "  -c --check      Request response and check for OK, or output error to stderr\n";
  cout << "  -r --response   Request response and output it to stdout\n";
  cout << "  -v --verbose    More logging\n";
  cout << "  -q --quiet      No logging, even on error\n";
  cout << "  -h --host       Set XMLMesh host (default 'localhost')\n";
  cout << "  -p --port       Set XMLMesh port (default " 
       << XMLMesh::OTMP::DEFAULT_PORT << ")\n";
  cout << "  -? --help       Output this usage\n";
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  bool check = false;
  bool show_response = false;

  string host("localhost");
  int port = XMLMesh::OTMP::DEFAULT_PORT;
  Log::Level log_level = Log::LEVEL_ERROR;

  if (argc < 2)
  {
    usage(argv[0]);
    return 0;
  }

  // Last argument is always subject
  string subject(argv[argc-1]);

  // Parse options
  for(int i=1; i<argc-1; i++)
  {
    string opt(argv[i]);
    if (opt == "-c" || opt == "--check")
      check = true;
    else if (opt == "-r" || opt == "--response")
      show_response = true;
    else if (opt == "-v" || opt == "--verbose")
    {
      log_level = Log::LEVEL_DETAIL;
      OBTOOLS_LOG_IF_DEBUG(log_level = Log::LEVEL_DEBUG;)
    }
    else if (opt == "-q" || opt == "--quiet")
      log_level = Log::LEVEL_NONE;
    else if ((opt == "-h" || opt == "--host") && i<argc-2)
      host = argv[++i];
    else if ((opt == "-p" || opt == "--port") && i<argc-2)
      port = atoi(argv[++i]);
    else if (opt == "-?" || opt == "--help")
    {
      usage(argv[0]);
      return 0;
    }
    else
    {
      cerr << "Unknown option: " << opt << endl;
      usage(argv[0]);
      return 2;
    }
  }

  // Set up logging
  Log::StreamChannel   chan_out(cout);
  Log::LevelFilter     level_out(log_level, chan_out);
  Log::logger.connect(level_out);
  Log::Streams log;

  // Resolve name
  Net::IPAddress addr(host);
  if (!addr)
  {
    cerr << "Can't resolve host: " << host << endl;
    return 1;
  }

  log.summary << "Host: " << addr 
	      << " (" << addr.get_hostname() << ")" << endl;

  // Start client
  Net::EndPoint server(addr, port);
  XMLMesh::OTMPClient client(server);

  // Read message from stdin
  string xml;
  while (cin) cin >> xml;

  log.detail << "Subject: " << subject << endl;
  log.detail << "Message:\n" << xml << endl;
  if (show_response || check)
    log.detail << "Response requested: Yes\n";

  // Send it
  if (show_response || check)
  {
    XMLMesh::Message request(subject, xml, true);

    if (show_response)
    {
      XMLMesh::Message response;
      if (client.request(request, response))
	cout << response.get_text();
      else
	return 1;
    }
    else
    {
      if (!client.request(request)) return 1;
    }
  }
  else
  {
    XMLMesh::Message msg(subject, xml);
    if (!client.send(msg)) return 2;
  }

  return 0;  
}





