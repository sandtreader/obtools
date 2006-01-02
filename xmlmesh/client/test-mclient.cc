//==========================================================================
// ObTools::XMLMesh: test-mclient.cc
//
// Test harness for XMLMesh multi-threaded client
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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
    Log::Stream detail_log(Log::logger, Log::LEVEL_DETAIL);
    detail_log << "INFO:\n" << msg.get_text() << endl;
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
  XMLMesh::OTMPMultiClient client(server);

  // Subscribe to information
  InfoSubscriber *infosub = new InfoSubscriber(client); 
  
  // Loop for a while sending out messages
  for(int i=0; i<10; i++)
  {
    MT::Thread::sleep(1);
    XMLMesh::Message msg("info.foo", "<info>Boo!</info>");
    client.send(msg);
  }

  // Sleep while stuff comes back
  MT::Thread::sleep(5);

  // Disconnect, rather than delete
  infosub->disconnect();

  return 0;  
}




