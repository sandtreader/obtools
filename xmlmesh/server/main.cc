//==========================================================================
// ObTools::XMLBus:Server: main.cc
//
// Main entry point for XMLBus Server
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "server.h"
#include "ot-log.h"
#include "ot-xmlbus-otmp.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::XMLBus;

//--------------------------------------------------------------------------
// Send handler thread class
// Pulls messages off the given queue and sends them to the given socket
class ReflectorThread: public MT::Thread
{
  OTMP::Server& server;
  MT::Queue<OTMP::ClientMessage>& receive_q;

  void run() 
  { 
    for(;;)
    {
      // Block for a message
      OTMP::ClientMessage msg = receive_q.wait();

      // Send it back
      server.send(msg);
    }
  }

public:
  ReflectorThread(OTMP::Server &s, 
		  MT::Queue<OTMP::ClientMessage>& q): 
    server(s), receive_q(q) { start(); }
};

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  // Set up logging
  Log::StreamChannel   chan_out(cout);
  Log::TimestampFilter tsfilter("%H:%M:%S %a %d %b %Y: ", chan_out);
  Log::LevelFilter     level_out(Log::LEVEL_DUMP, tsfilter);
  Log::logger.connect(level_out);

  // Create unified receive queue
  MT::Queue<OTMP::ClientMessage> q;

  // Create server 
  OTMP::Server server(q);

  // Start reflector thread
  ReflectorThread reflector(server, q);

  // Run the server
  server.run();
  return 0;  
}




