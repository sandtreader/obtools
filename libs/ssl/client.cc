//==========================================================================
// ObTools::SSL: client.cc
//
// C++ wrapper for SSL client
//
// Copyright (c) 2008 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-ssl.h"
#include "ot-log.h"

namespace ObTools { namespace SSL {

//--------------------------------------------------------------------------
// Constructor
TCPClient::TCPClient(Context *ctx, Net::EndPoint endpoint):
  server(endpoint)
{
  // Connect ordinary TCPClient, check its OK and steal its fd
  Net::TCPClient base(endpoint);
  connected = !!base;
  fd = base.detach_fd();
  attach_ssl(ctx);
}

// Note: All other constructors are basically identical mimics of their
// base counterparts

//--------------------------------------------------------------------------
// Constructor with a timeout on connection (in seconds)
TCPClient::TCPClient(Context *ctx, Net::EndPoint endpoint, int timeout):
  server(endpoint)
{
  Net::TCPClient base(endpoint, timeout);
  connected = !!base;
  fd = base.detach_fd();
  attach_ssl(ctx);
}

//--------------------------------------------------------------------------
// Constructor, binding specific local address/port
// port can be zero if you only want to bind address
TCPClient::TCPClient(Context *ctx, Net::EndPoint local, Net::EndPoint remote):
  server(remote)
{
  Net::TCPClient base(local, remote);
  connected = !!base;
  fd = base.detach_fd();
  attach_ssl(ctx);
}

//--------------------------------------------------------------------------
// Constructor, binding specific local address/port and with timeout
// port can be zero if you only want to bind address
TCPClient::TCPClient(Context *ctx, Net::EndPoint local, Net::EndPoint remote,
		     int timeout): server(remote)
{
  Net::TCPClient base(local, remote, timeout);
  connected = !!base;
  fd = base.detach_fd();
  attach_ssl(ctx);
}

//--------------------------------------------------------------------------
// Constructor, binding specific local address/port and with timeout and TTL
// port can be zero if you only want to bind address
TCPClient::TCPClient(Context *ctx, Net::EndPoint local, Net::EndPoint remote,
		     int timeout, int ttl): server(remote)
{
  Net::TCPClient base(local, remote, timeout, ttl);
  connected = !!base;
  fd = base.detach_fd();
  attach_ssl(ctx);
}

//--------------------------------------------------------------------------
// Constructor from existing fd
TCPClient::TCPClient(Context *ctx, int _fd, Net::EndPoint remote):
  server(remote)
{
  fd = _fd;
  connected = true;
  attach_ssl(ctx);
}

//--------------------------------------------------------------------------
// Attach SSL to the fd
void TCPClient::attach_ssl(Context *ctx)
{
  if (ctx && connected)
  {
    ssl = ctx->connect_connection(fd);
    if (!ssl) close();
  }
  else ssl = 0;
}

}} // namespaces



