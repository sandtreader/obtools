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
// Constructor, allocating any local address/port
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

//--------------------------------------------------------------------------
// Constructor, binding specific local address/port
// port can be zero if you only want to bind address
TCPClient::TCPClient(EndPoint local, EndPoint remote):
  TCPSocket(),
  server(remote),
  connected(false)
{
  struct sockaddr_in saddr;
  server.set(saddr);

  // Bind local side first
  if (fd != INVALID_FD && bind(local)
      && !connect(fd, (struct sockaddr *)&saddr, sizeof(saddr)))
    connected = true;
}



}} // namespaces



