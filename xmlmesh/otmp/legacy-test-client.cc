//==========================================================================
// ObTools::XMLMesh: test-client.cc
//
// Test harness for raw OTMP client
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xmlmesh-otmp.h"
#include "ot-log.h"
#include <unistd.h>
#include <stdlib.h>

#if defined(PLATFORM_WINDOWS)
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

#if defined(PLATFORM_WINDOWS)
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
  XMLMesh::OTMP::Client client(server);

  // Loop for a while sending and receiving
  for(int i=0; i<10; i++)
  {
    this_thread::sleep_for(chrono::seconds{1});

    XMLMesh::OTMP::Message msg("This is a test message");

    client.send(msg);

#if !defined(_SINGLE)
    if (client.poll())
#endif
    {
      if (client.wait(msg))
        cout << msg.data << endl;
      else
        cout << "RESTART\n";
    }
  }

  cout << "Shutting down\n";
  client.shutdown();
  cout << "Done\n";

  return 0;
}




