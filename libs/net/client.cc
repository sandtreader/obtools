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
TCP_Client::TCP_Client(IP_Address addr, int port):
  server_addr(addr),
  server_port(port),
  connected(false)
{
  struct sockaddr_in serv_addr;

  /* Connect with automatic bind (any old port this end) */
  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_addr.s_addr = server_addr.nbo();
  serv_addr.sin_port        = htons(server_port);

  if (fd>=0 && !connect(fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)))
    connected = true;
}



}} // namespaces



