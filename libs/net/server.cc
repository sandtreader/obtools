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
// Run server
// doesn't return unless it all falls apart.
void TCPServer::run()
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
      IPAddress client_address(htonl(saddr.sin_addr.s_addr));
      int client_port = ntohs(saddr.sin_port);

      // Get a thread
      TCPServerThread *thread = threadpool.remove();
      if (thread)
      {
	// Fill in parameters
	thread->server         = this;
	thread->client_fd      = new_fd;
	thread->client_address = client_address;
	thread->client_port    = client_port;

	// Start it off
	thread->kick();
      }
      else
      {
	// Dump it
	::close(new_fd);
      }
    }
  }
}

//--------------------------------------------------------------------------
// Server thread 'run' function
void TCPServerThread::run()
{
  // Create wrapped socket which will also close on exit
  TCPSocket s(client_fd);

  // Just pass them to the server's process function
  server->process(s, client_address, client_port);
}

}} // namespaces



