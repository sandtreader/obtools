//==========================================================================
// ObTools::Tube: test-bi-sync-server.cc
//
// Test harness for tube bidirectional sync request server
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-tube.h"
#include <stdlib.h>

using namespace std;
using namespace ObTools;

Net::EndPoint nowhere;
SSL::ClientDetails client_details(nowhere, "");

//--------------------------------------------------------------------------
// Background send thread class
class TestThread: public MT::Thread
{
  Tube::BiSyncServer& server;

  void run() 
  { 
    for(;;)
    {
      if (!!client_details.address.host)
      {
	Tube::ClientMessage request(client_details, 0x484C4C4F, "Hello!");
	Tube::Message response;
	server.request(request, response);
      }
      MT::Thread::sleep(1);
    }
  }

public:
  TestThread(Tube::BiSyncServer &_server): server(_server) { start(); }
};

//--------------------------------------------------------------------------
// Test server class
class TestServer: public Tube::BiSyncServer
{
private:
  bool handle_request(Tube::ClientMessage& msg, Tube::Message& response)
  {
    response = msg.msg;  // Reflect it once
    client_details = msg.client;       // Capture to send back

    // Delay
    MT::Thread::sleep(3);

    return true;
  }

public:
  TestServer(int port): Tube::BiSyncServer(port) {}
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

  // Start test thread
  TestThread test_thread(server);

  // Run the server
  server.run();
  return 0;  
}




