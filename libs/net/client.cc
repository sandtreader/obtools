//==========================================================================
// ObTools::Net: client.cc
//
// TCP and UDP clients
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-net.h"

namespace ObTools { namespace Net {

//==========================================================================
// TCP client

//--------------------------------------------------------------------------
// Constructor, allocating any local address/port and connecting to server
TCPClient::TCPClient(EndPoint endpoint):
  TCPSocket(),
  server(endpoint),
  connected(false)
{
  struct sockaddr_in saddr;
  server.set(saddr);

  if (fd != INVALID_FD
   && !connect(fd, reinterpret_cast<struct sockaddr *>(&saddr), sizeof(saddr)))
    connected = true;
}

//--------------------------------------------------------------------------
// Constructor with a timeout on connection (in seconds)
TCPClient::TCPClient(EndPoint endpoint, int timeout):
  TCPSocket(),
  server(endpoint),
  connected(false)
{
  struct sockaddr_in saddr;
  server.set(saddr);

  if (fd != INVALID_FD)
  {
    set_timeout(timeout);
    if (!connect(fd, reinterpret_cast<struct sockaddr *>(&saddr),
                 sizeof(saddr)))
      connected = true;
    set_timeout(0);  // Cancel timeout for operation
  }
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

  if (fd!=INVALID_FD)
  {
    // Set REUSEADDR to force grab of local socket
    enable_reuse();

    // Bind local side first
    if (bind(local)
        && !connect(fd, reinterpret_cast<struct sockaddr *>(&saddr),
                    sizeof(saddr)))
      connected = true;
  }
}

//--------------------------------------------------------------------------
// Constructor, binding specific local address/port and with timeout
// port can be zero if you only want to bind address
TCPClient::TCPClient(EndPoint local, EndPoint remote, int timeout):
  TCPSocket(),
  server(remote),
  connected(false)
{
  struct sockaddr_in saddr;
  server.set(saddr);

  if (fd!=INVALID_FD)
  {
    set_timeout(timeout);

    // Set REUSEADDR to force grab of local socket
    enable_reuse();

    // Bind local side first
    if (bind(local) && !connect(fd,
                                reinterpret_cast<struct sockaddr *>(&saddr),
                                sizeof(saddr)))
        connected = true;

    set_timeout(0);  // Cancel timeout for operation
  }
}

//--------------------------------------------------------------------------
// Constructor, binding specific local address/port and with timeout and TTL
// port can be zero if you only want to bind address
TCPClient::TCPClient(EndPoint local, EndPoint remote, int timeout, int ttl):
  TCPSocket(),
  server(remote),
  connected(false)
{
  struct sockaddr_in saddr;
  server.set(saddr);

  if (fd!=INVALID_FD)
  {
    set_timeout(timeout);
    set_ttl(ttl);

    // Set REUSEADDR to force grab of local socket
    enable_reuse();

    // Bind local side first
    if (bind(local) && !connect(fd,
                                reinterpret_cast<struct sockaddr *>(&saddr),
                                sizeof(saddr)))
        connected = true;

    set_timeout(0);  // Cancel timeout for operation
  }
}

//--------------------------------------------------------------------------
// Constructor from existing fd
TCPClient::TCPClient(int _fd, EndPoint remote):
  TCPSocket(_fd), server(remote), connected(true)
{

}

}} // namespaces



