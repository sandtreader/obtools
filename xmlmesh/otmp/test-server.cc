//==========================================================================
// ObTools::XMLBus: test-server.cc
//
// Test harness for raw OTMP server
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmlbus-otmp.h"
#include "ot-log.h"

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Send handler thread class
// Pulls messages off the given queue and sends them to the given socket
class ReflectorThread: public MT::Thread
{
  XMLBus::OTMP::Server& server;
  MT::Queue<XMLBus::OTMP::Message>& receive_q;

  void run() 
  { 
    for(;;)
    {
      // Block for a message
      XMLBus::OTMP::Message msg = receive_q.wait();

      // Send it back
      server.send(msg);
    }
  }

public:
  ReflectorThread(XMLBus::OTMP::Server &s, 
		  MT::Queue<XMLBus::OTMP::Message>& q): 
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
  MT::Queue<XMLBus::OTMP::Message> q;

  // Create server 
  XMLBus::OTMP::Server server(q);

  // Start reflector thread
  ReflectorThread reflector(server, q);

  // Run the server
  server.run();
  return 0;  
}




