//==========================================================================
// ObTools::Tube: test-sync-server.cc
//
// Test harness for tube sync request server
//
// Copyright (c) 2007 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-tube.h"
#include <stdlib.h>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Test server class
class TestServer: public Tube::SyncServer
{
private:
  bool handle_request(const Tube::ClientMessage& msg,
                      Tube::Message& response)
  {
    // Send it back as response
    response = msg.msg;

    // Also send it back as an async message
    //    send(msg);

    return true;
  }

public:
  TestServer(int port): Tube::SyncServer(port) {}
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




