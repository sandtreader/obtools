//==========================================================================
// ObTools::XMLMesh: test-mclient.cc
//
// Test harness for XMLMesh multi-threaded client
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmlmesh-client-otmp.h"
#include "ot-log.h"
#include <unistd.h>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Info subscriber class
class InfoSubscriber: public XMLMesh::Subscriber
{
public:
  InfoSubscriber(XMLMesh::MultiClient& client): 
    XMLMesh::Subscriber(client, "info.*") {}

  void handle(XMLMesh::Message& msg)
  {
    Log::Detail << "INFO:\n" << msg.get_text() << endl;
  }
};

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
  XMLMesh::OTMPMultiClient client(server);

  // Subscribe to information
  InfoSubscriber infosub(client); 
  
  // Loop for a while sending out messages
  for(int i=0; i<10; i++)
  {
    sleep(1);
    XMLMesh::Message msg("info.foo", "<info>Boo!</info>");
    client.send(msg);
  }

  // Sleep while stuff comes back
  sleep(5);

  return 0;  
}




