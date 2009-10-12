//==========================================================================
// ObTools::Tube: test-client.cc
//
// Test harness for tube client
//
// Copyright (c) 2007 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-tube.h"
#include <stdlib.h>

#if defined(__WIN32__)
#include <windows.h>
#endif

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    cerr << "Give a hostname and port\n" << endl;
    return 2;
  }

  char *host = argv[1];
  int port = atoi(argv[2]);

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
  Tube::Client client(server);

  // Loop for a while sending and receiving
  for(int i=0; i<10; i++)
  {
    MT::Thread::sleep(1);
    Tube::Message msg(0x12345678, "This is a test message");

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




