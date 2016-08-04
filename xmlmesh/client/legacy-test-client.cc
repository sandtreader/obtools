//==========================================================================
// ObTools::XMLMesh: test-client.cc
//
// Test harness for XMLMesh client
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xmlmesh-client-otmp.h"
#include "ot-log.h"
#include <unistd.h>

#if defined(__WIN32__)
#include <windows.h>
#endif

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cerr << "Give a hostname and optional port\n" << endl;
    return 2;
  }

  char *host = argv[1];
  int port = XMLMesh::OTMP::DEFAULT_PORT;
  if (argc > 2) port = atoi(argv[2]);

  // Set up logging
  Log::StreamChannel   chan_out(cout);
  Log::TimestampFilter tsfilter("%H:%M:%S %a %d %b %Y: ", chan_out);
  Log::LevelFilter     level_out(Log::LEVEL_DUMP, tsfilter);
  Log::logger.connect(level_out);
  Log::Streams log;

#ifdef __WIN32__
  winsock_initialise();
#endif

  // Resolve name
  Net::IPAddress addr(host);
  if (!addr)
  {
    log.error << "Can't resolve host: " << host << endl;
    return 1;
  }

  log.summary << "Host: " << addr
	      << " (" << addr.get_hostname() << ")" << endl;

  // Start client
  Net::EndPoint server(addr, port);
  XMLMesh::OTMPClient client(server);

  // Subscribe to some stuff
  XMLMesh::Subscription sub(client, "info.*");

  // Loop for a while sending and receiving
  for(int i=0; i<30; i++)
  {
    this_thread::sleep_for(chrono::seconds{1});
    XMLMesh::Message msg("info.foo", "<info>Boo!</info>");

    client.send(msg);

#if defined(_SINGLE)
    if (client.wait(msg))
#else
    if (client.poll(msg))
#endif
      log.detail << msg.get_text() << endl;
  }

  return 0;
}




