//==========================================================================
// ObTools::XMLMesh: test-client.cc
//
// Test harness for XMLMesh client
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#include "ot-xmlmesh-client-otmp.h"
#include "ot-log.h"
#include <unistd.h>

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

  // Resolve name
  Net::IPAddress addr(host);
  if (!addr)
  {
    Log::Error << "Can't resolve host: " << host << endl;
    return 1;
  }

  Log::Summary << "Host: " << addr << 
    " (" << addr.get_hostname() << ")" << endl;

  // Start client
  Net::EndPoint server(addr, port);
  XMLMesh::OTMPClient client(server);

  // Subscribe to some stuff
  XMLMesh::Subscription sub(client, "info.*");
  
  // Loop for a while sending and receiving
  for(int i=0; i<30; i++)
  {
    sleep(1);
    XMLMesh::Message msg("info.foo", "<info>Boo!</info>");

    client.send(msg);

    if (client.poll(msg)) 
      Log::Detail << msg.get_text() << endl;
  }

  return 0;  
}




