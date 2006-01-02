//==========================================================================
// ObTools::Net: client.cc
//
// TCP and UDP clients
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-net.h"

namespace ObTools { namespace Net {

//==========================================================================
// TCP client

//--------------------------------------------------------------------------
// Constructor 
TCPClient::TCPClient(EndPoint endpoint):
  TCPSocket(),
  server(endpoint),
  connected(false)
{
  struct sockaddr_in saddr;
  server.set(saddr);

  if (fd != INVALID_FD 
   && !connect(fd, (struct sockaddr *)&saddr, sizeof(saddr)))
    connected = true;
}



}} // namespaces



