//==========================================================================
// ObTools::XMLMesh: test-server.cc
//
// Test harness for raw OTMP server
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-xmlmesh-otmp.h"
#include "ot-log.h"
#include <stdlib.h>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Send handler thread class
// Pulls messages off the given queue and sends them to the given socket
class ReflectorThread: public MT::Thread
{
  XMLMesh::OTMP::Server& server;
  MT::Queue<XMLMesh::OTMP::ClientMessage>& receive_q;

  void run()
  {
    for(;;)
    {
      // Block for a message
      XMLMesh::OTMP::ClientMessage msg = receive_q.wait();

      // Send it back
      if (msg.action == XMLMesh::OTMP::ClientMessage::MESSAGE_DATA)
	server.send(msg);
    }
  }

public:
  ReflectorThread(XMLMesh::OTMP::Server &s,
		  MT::Queue<XMLMesh::OTMP::ClientMessage>& q):
    server(s), receive_q(q) { start(); }
};

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  int port = XMLMesh::OTMP::DEFAULT_PORT;
  if (argc > 1) port = atoi(argv[1]);

#ifdef __WIN32__
  winsock_initialise();
#endif

  // Set up logging
  Log::StreamChannel   chan_out(cout);
  Log::TimestampFilter tsfilter("%H:%M:%S %a %d %b %Y: ", chan_out);
  Log::LevelFilter     level_out(Log::LEVEL_DUMP, tsfilter);
  Log::logger.connect(level_out);

  // Create unified receive queue
  MT::Queue<XMLMesh::OTMP::ClientMessage> q;

  // Create server
  XMLMesh::OTMP::Server server(q, port);
  server.open();

  // Start reflector thread
  ReflectorThread reflector(server, q);

  // Run the server
  server.run();
  return 0;
}




