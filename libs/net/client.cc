//==========================================================================
// ObTools::Net: client.cc
//
// TCP and UDP clients
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-net.h"

namespace ObTools { namespace Net {

//==========================================================================
// TCP client

//--------------------------------------------------------------------------
// Constructor 
TCPClient::TCPClient(IPAddress addr, int port):
  server_addr(addr),
  server_port(port),
  connected(false)
{
  struct sockaddr_in saddr;

  saddr.sin_family      = AF_INET;
  saddr.sin_addr.s_addr = server_addr.nbo();
  saddr.sin_port        = htons(server_port);

  if (fd>=0 && !connect(fd, (struct sockaddr *)&saddr, sizeof(saddr)))
    connected = true;
}

//--------------------------------------------------------------------------
// Destructor
TCPClient::~TCPClient()
{
  Socket::close();
}


}} // namespaces



