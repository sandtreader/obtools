//==========================================================================
// ObTools::Net: single-server.cc
//
// Single-threaded TCP server
//
// Copyright (c) 2007 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-net.h"

namespace ObTools { namespace Net {

//--------------------------------------------------------------------------
// Start server listening
void TCPSingleServer::start()
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
  if (::listen(fd, backlog)) TCPSocket::close();
}

//--------------------------------------------------------------------------
// Listen for a connection and return a TCP socket
// If timeout is non-zero, times out and returns 0 if no connection in 
// that time
// Returns connected socket or 0 if it fails
TCPSocket *TCPSingleServer::wait(int timeout)
{
  if (fd == INVALID_FD) return 0;

  // If timeout specified, select for accept
  if (timeout && !wait_readable(timeout)) return 0;

  // Accept connection
  fd_t new_fd = ::accept(fd, 0, 0);
  if (new_fd != INVALID_FD) 
    return new TCPSocket(new_fd);
  else 
    return 0;
}


}} // namespaces



