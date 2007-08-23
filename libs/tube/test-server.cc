//==========================================================================
// ObTools::Tube: test-server.cc
//
// Test harness for tube server
//
// Copyright (c) 2007 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-tube.h"

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Test server class
class TestServer: public Tube::Server
{
private:
  bool handle_message(Tube::ClientMessage& msg)
  {
    // Send it back
    if (msg.action == Tube::ClientMessage::MESSAGE) send(msg);

    return true;
  }

public:
  TestServer(int port): Tube::Server(port) {}
};

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cerr << "Give a port\n" << endl;
    return 2;
  }

  int port = atoi(argv[1]);

#ifdef __WIN32__
  winsock_initialise();
#endif

  // Set up logging
  Log::StreamChannel   chan_out(cout);
  Log::TimestampFilter tsfilter("%H:%M:%S %a %d %b %Y: ", chan_out);
  Log::LevelFilter     level_out(Log::LEVEL_DUMP, tsfilter);
  Log::logger.connect(level_out);

  // Create server 
  TestServer server(port);
  server.open();

  // Run the server
  server.run();
  return 0;  
}




