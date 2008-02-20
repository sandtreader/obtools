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
  enable_reuse();

  // Bind to local port (this is Socket::bind()), specifying address
  // (which might be INADDR_ANY from our constructor
  if (!bind(address))
  {
    TCPSocket::close();
    return;
  }

  // Start listing with backlog
  if (::listen(fd, backlog))
  {
    TCPSocket::close();
    return;
  }

  // Now loop accepting connections into new threads
  while (alive)
  {
    struct sockaddr_in saddr;
    socklen_t len = sizeof(saddr);

    // Get a thread before we accept, so we know we can handle the
    // resulting connection - this forces overload connections into the
    // backlog
    TCPWorkerThread *thread = threadpool.wait();

    fd_t new_fd = ::accept(fd, (struct sockaddr *)&saddr, &len);
    if (alive && new_fd != INVALID_FD) 
    {
      EndPoint client(saddr);

      // Check it's allowed - we do this as soon as possible to prevent
      // any userland DoS through nobbling all the threads
      // Any kernel susceptibility to DoS remains, of course - if only
      // there was a way to pass an allowed-list to the kernel...
      if (!verify(client))
      {
	::SOCKCLOSE(new_fd);
	threadpool.replace(thread);
	continue;
      }

      // Fill in parameters
      thread->server         = this;
      thread->client_fd      = new_fd;
      thread->client_ep      = client;
	
      // Start it off
      thread->kick();
    }
    else threadpool.replace(thread);
  }
}

//--------------------------------------------------------------------------
// Initiate an outgoing connection, from the same local address as we use
// for serving, and then treat it as if it was an incoming one - mainly for P2P
// Connection is run with a worker thread just like an incoming connection
// Timeout is in seconds
// Returns fd of connection
Socket::fd_t TCPServer::initiate(EndPoint remote_address, int timeout)
{
  // Get a thread first
  TCPWorkerThread *thread = threadpool.wait();

  // Try to connect
  TCPClient client(address, remote_address, timeout);

  if (!!client)
  {
    fd_t fd = client.detach_fd();

    thread->server         = this;
    thread->client_fd      = fd;
    thread->client_ep      = remote_address;
	
    // Start it off
    thread->kick();
    return fd;
  }
  else 
  {
    threadpool.replace(thread);
    return INVALID_FD;
  }
}

//--------------------------------------------------------------------------
// Accept an existing socket into the server to be processed
// Used for P2P where 'server' socket may be initiated at this end
void TCPServer::take_over(int fd, Net::EndPoint remote_address)
{
  // Get a thread first
  TCPWorkerThread *thread = threadpool.wait();

  thread->server         = this;
  thread->client_fd      = fd;
  thread->client_ep      = remote_address;
	
  // Start it off
  thread->kick();
}
    
//--------------------------------------------------------------------------
// Shut down server
void TCPServer::shutdown()
{
  if (alive)
  {
    alive = false;
    close();
    threadpool.shutdown(); 
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



