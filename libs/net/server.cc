//==========================================================================
// ObTools::Net: server.cc
//
// Multithreaded TCP server
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-net.h"

namespace ObTools { namespace Net {

//--------------------------------------------------------------------------
// Constructor.  Starts listening immediately and doesn't return unless
// it all falls apart.
TCPServer::TCPServer(int _port, int _backlog):
  port(_port), backlog(_backlog)
{
  if (fd < 0) return;

  // Set REUSEADDR for fast restarts (e.g. during development)
  int one = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));

  // Bind to local port, allow any remote address or port 
  struct sockaddr_in saddr;
  saddr.sin_family      = AF_INET;
  saddr.sin_addr.s_addr = INADDR_ANY;
  saddr.sin_port        = htons(port);

  if (bind(fd, (struct sockaddr *)&saddr, sizeof(saddr))) return;

  // Start listing with backlog
  if (listen(fd, backlog)) return;

  // Now loop accepting connections into new threads
  while (1)
  {
    socklen_t len = sizeof(saddr);
    int new_fd = accept(fd, (struct sockaddr *)&saddr, &len);
    if (new_fd >= 0)
    {
      TCPSocket client_socket(new_fd);
      IPAddress client_address(htonl(saddr.sin_addr.s_addr));
      int client_port = ntohs(saddr.sin_port);

      //!!! Hand it to thread pool
    }
  }
}



}} // namespaces



