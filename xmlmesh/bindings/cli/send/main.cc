// =======================================================================
// ObTools::XMLMesh: send/main.cc
//
// Command-line interface to XMLMesh messaging
//
// Accepts message to send on stdin, and may also output response on stdout
// - see usage() below
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
// ======================================================================= 

#include "ot-xmlmesh-client-otmp.h"
#include "ot-log.h"
#include <fstream>
#include <errno.h>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Usage
void usage(char *pname)
{
  cout << "ObTools XMLMesh command line message interface\n\n";
  cout << "Usage:\n";
  cout << "  " << pname << " [options] <subject> [<file>]\n\n";
  cout << "Reads message from <file> or stdin, and sends it with the given subject\n";
  cout << "May output response to stdout if requested\n";
  cout << "Result code 0 for success, 1 for message failure, 2 for fatal error\n\n";
  cout << "Options:\n";
  cout << "  -c --check      Request response and check for OK, or output error to stderr\n";
  cout << "  -r --response   Request response and output body to stdout\n";
  cout << "  -s --soap       Show full SOAP response (only if -r)\n";
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
  if (argc < 2)
  {
    usage(argv[0]);
    return 0;
  }

  string subject;
  string file;
  bool check = false;
  bool show_response = false;
  bool soap_response = false;
  string host("localhost");
  int port = XMLMesh::OTMP::DEFAULT_PORT;
  Log::Level log_level = Log::LEVEL_ERROR;

  // Parse options
  for(int i=1; i<argc; i++)
  {
    string opt(argv[i]);

    if (opt[0] == '-')
    {
      if (opt == "-c" || opt == "--check")
	check = true;
      else if (opt == "-r" || opt == "--response")
	show_response = true;
      else if (opt == "-s" || opt == "--soap")
	soap_response = true;
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
    else if (subject.empty())
      subject = opt;
    else if (file.empty())
      file = opt;
    else
    {
      cerr << "Extra arguments ignored\n";
      break;
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

  // Read message from file or stdin
  string xml;
  if (file.empty())
  {
    while (cin)
    {
      char c;
      cin.get(c);
      xml+=c;
    }
  }
  else
  {
    ifstream fin(file.c_str());
    if (!fin) 
    {
      cerr << "Can't read file " << file << endl;
      return 2;
    }
    while (fin) 
    {
      char c;
      fin.get(c);
      xml+=c;
    }
  }

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
      {
	if (soap_response)
	  cout << response.get_text();
	else
	  cout << response.get_body();
      }
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
    this_thread::sleep_for(chrono::seconds{1});  // Wait for it to go out
  }

  return 0;  
}





