//==========================================================================
// ObTools::CLI: ot-cli-telnet.h
//
// Public definitions for Telnet (TCP) command line
// 
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_CLI_TELNET_H
#define __OBTOOLS_CLI_TELNET_H

#include "ot-cli.h"
#include "ot-net.h"

namespace ObTools { namespace CLI { 

//Make our lives easier without polluting anyone else
using namespace std;
using namespace ObTools;

//==========================================================================
// Telnet Command Line server
class TelnetServer: public Net::TCPServer
{
  Registry& registry;

public:
  string prompt;

  //------------------------------------------------------------------------
  //Constructor
  TelnetServer(Registry& _registry, int port, const string& _prompt=">"): 
    registry(_registry), prompt(_prompt), Net::TCPServer(port) {}

  //------------------------------------------------------------------------
  //Process a connection
  void process(Net::TCPSocket& s, Net::EndPoint client);
};



//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_CLI_TELNET_H



