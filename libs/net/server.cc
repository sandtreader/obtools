//==========================================================================
// ObTools::Net: server.cc
//
// Multithreaded TCP server
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-net.h"

#ifdef __WIN32__
#include <io.h>
#define SOCKCLOSE closesocket
#else
#define SOCKCLOSE close
#endif

namespace ObTools { namespace Net {

//--------------------------------------------------------------------------
// Run server
// doesn't return unless it all falls apart.
void TCPServer::run()
{
  if (fd == INVALID_FD) return;

  // Set REUSEADDR for fast restarts (e.g. during development)
  int one = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(int));

  // Bind to local port (this is Socket::bind())
  if (!bind(port)) return;

  // Start listing with backlog
  if (::listen(fd, backlog)) return;

  // Now loop accepting connections into new threads
  while (1)
  {
    struct sockaddr_in saddr;
    socklen_t len = sizeof(saddr);
    fd_t new_fd = ::accept(fd, (struct sockaddr *)&saddr, &len);
    if (new_fd != INVALID_FD)
    {
      EndPoint client(saddr);

      // Check it's allowed - we do this as soon as possible to prevent
      // any userland DoS through nobbling all the threads
      // Any kernel susceptibility to DoS remains, of course - if only
      // there was a way to pass an allowed-list to the kernel...
      if (!verify(client))
      {
	::SOCKCLOSE(new_fd);
	continue;
      }

      // Get a thread
      TCPWorkerThread *thread = threadpool.remove();
      if (thread)
      {
	// Fill in parameters
	thread->server         = this;
	thread->client_fd      = new_fd;
	thread->client_ep      = client;

	// Start it off
	thread->kick();
      }
      else
      {
	// Dump it
	::SOCKCLOSE(new_fd);
      }
    }
  }
}

//--------------------------------------------------------------------------
// Worker thread 'run' function
void TCPWorkerThread::run()
{
  // Create wrapped socket which will also close on exit
  TCPSocket s(client_fd);

  // Just pass them to the server's process function
  server->process(s, client_ep);
}

}} // namespaces



