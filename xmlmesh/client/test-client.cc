//==========================================================================
// ObTools::XMLBus: test-client.cc
//
// Test harness for XMLBus client
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmlbus.h"
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
  int port = 0;
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
  XMLBus::Client client(addr, port);

  // Loop for a while sending and receiving
  for(int i=0; i<30; i++)
  {
    sleep(1);
    XMLBus::Message msg("Test", "This is a test message");

    client.send(msg);

    if (client.poll(msg))
    {
      cout << msg.subject << ": " << msg.content << endl;
    }
  }

  return 0;  
}




