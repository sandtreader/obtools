//==========================================================================
// ObTools::Net: client.cc
//
// TCP and UDP clients
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#include "ot-net.h"

namespace ObTools { namespace Net {

//==========================================================================
// TCP client

//--------------------------------------------------------------------------
// Constructor 
TCPClient::TCPClient(EndPoint endpoint):
  server(endpoint),
  connected(false)
{
  struct sockaddr_in saddr;
  server.set(saddr);

  if (fd>=0 && !connect(fd, (struct sockaddr *)&saddr, sizeof(saddr)))
    connected = true;
}



}} // namespaces



