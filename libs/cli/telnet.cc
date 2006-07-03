//==========================================================================
// ObTools::CLI: telnet.cc
//
// Implementation of Telnet (TCP) CLI
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-cli-telnet.h"
#include <sstream>

namespace ObTools { namespace CLI {

//------------------------------------------------------------------------
//Process a connection
void TelnetServer::process(Net::TCPSocket& s, Net::EndPoint)
{
  // Create streams for input/output
  Net::TCPStream io(s);

  try
  {
    io << prompt;
    io.flush();

    // Read lines and handle them
    string line;

    for(;;)
    {
      char c=0;
      io.get(c);

      switch (c)
      {
	case 0: 
	case 4:  // ctrl-D
	  return;

	case '\r':  // Ignore
	  break;

	case '\n':
	  if (line.size())
	  {
	    // Handle this line 
	    registry.handle(line, io, io);
	    line.erase();
	  }

	  io << prompt;
	  io.flush();
	break;

	default:
	  line+=c;
      }
    }
  }
  catch (Net::SocketError se)
  {
    cerr << se << endl;
  }
}

}} // namespaces
