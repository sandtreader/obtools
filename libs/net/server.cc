//==========================================================================
// ObTools::Net: server.cc
//
// Multithreaded TCP server
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-net.h"
#include <memory>

#ifdef __WIN32__
#include <io.h>
#define SOCKCLOSE closesocket
#else
#include <unistd.h>
#define SOCKCLOSE close
#endif

namespace ObTools { namespace Net {

//--------------------------------------------------------------------------
// Set up sockets for server
void TCPServer::start()
{
  if (fd == INVALID_FD)
  {
    alive = false;
    return;
  }

  // Set REUSEADDR for fast restarts (e.g. during development, and to avoid
  // delays/failure in server restarting)

  // Don't use on Windows because it's too powerful - allows double bind
  // to the same explicit port even if something else is actively listening
  // to it.
#ifndef __WIN32__
  enable_reuse();
#endif

  // Bind to local port (this is Socket::bind()), specifying address
  // (which might be INADDR_ANY from our constructor
  if (!bind(address))
  {
    TCPSocket::close();
    alive = false;
    return;
  }

  // Start listing with backlog
  if (::listen(fd, backlog))
  {
    TCPSocket::close();
    alive = false;
  }
}

//--------------------------------------------------------------------------
// Run server
// doesn't return unless it all falls apart.
void TCPServer::run()
{
  // Now loop accepting connections into new threads
  while (alive)
  {
    struct sockaddr_in saddr;
    socklen_t len = sizeof(saddr);

    // Get a thread before we accept, so we know we can handle the
    // resulting connection - this forces overload connections into the
    // backlog
    TCPWorkerThread *thread = threadpool.wait();
    if (!thread) break;  // only at shutdown

#ifdef __WIN32__
    fd_t new_fd = ::accept(fd, reinterpret_cast<struct sockaddr *>(&saddr),
                           &len);
#else
    fd_t new_fd = ::accept4(fd, reinterpret_cast<struct sockaddr *>(&saddr),
                            &len, SOCK_CLOEXEC);
#endif
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
  if (!thread) return INVALID_FD;

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
  if (!thread) return;

  thread->server         = this;
  thread->client_fd      = fd;
  thread->client_ep      = remote_address;

  // Start it off
  thread->kick();
}

//--------------------------------------------------------------------------
// Default factory for creating a client socket - just return a standard
// TCPSocket
TCPSocket *TCPServer::create_client_socket(int client_fd)
{
  return new TCPSocket(client_fd);
}

//--------------------------------------------------------------------------
// Shut down server
void TCPServer::shutdown()
{
  if (alive)
  {
    alive = false;
    Socket::shutdown();  // Force accept() to exit
    close();  // Close listen socket
  }

  // Note: Client sockets will be closed by TCPWorkerThread::die()
  threadpool.shutdown();
}

//--------------------------------------------------------------------------
// Worker thread 'run' function
void TCPWorkerThread::run()
{
  // Create wrapped socket which will also close on exit
  unique_ptr<TCPSocket> s(server->create_client_socket(client_fd));

  // Just pass them to the server's process function
  if (s.get())
    server->process(*s, client_ep);
  else
    ::SOCKCLOSE(client_fd);  // Drop it

  // Clear it so we don't try to close it again on die()
  client_fd = -1;
}

//--------------------------------------------------------------------------
// Force socket to close on being asked to die
void TCPWorkerThread::die(bool wait)
{
  if (client_fd >= 0)
  {
#if defined(__WIN32__)
    ::shutdown(client_fd, SD_BOTH);
#else
    ::shutdown(client_fd, SHUT_RDWR);
#endif
    ::SOCKCLOSE(client_fd);
  }

  MT::PoolThread::die(wait);
}

}} // namespaces



