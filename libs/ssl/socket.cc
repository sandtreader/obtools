//==========================================================================
// ObTools::SSL: socket.cc
//
// C++ wrapper for SSL sockets
//
// Copyright (c) 2008 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-ssl.h"
#include "ot-log.h"

namespace ObTools { namespace SSL {

//--------------------------------------------------------------------------
// Raw stream read wrapper
ssize_t TCPSocket::cread(void *buf, size_t count)
{ 
  // If not SSL, revert to basic
  if (!ssl) return Net::TCPSocket::cread(buf, count);

  return ssl->cread(buf, count);
}

//--------------------------------------------------------------------------
// Raw stream write wrapper
ssize_t TCPSocket::cwrite(const void *buf, size_t count)
{ 
  // If not SSL, revert to basic
  if (!ssl) return Net::TCPSocket::cwrite(buf, count);

  return ssl->cwrite(buf, count);
}

//--------------------------------------------------------------------------
// Get peer's X509 common name
string TCPSocket::get_peer_cn()
{
  if (!ssl) return "";  // Invalid
  return ssl->get_peer_cn();
}

//--------------------------------------------------------------------------
// Destructor
TCPSocket::~TCPSocket()
{
  // Shutdown and free SSL connection
  if (ssl) delete ssl;
}

}} // namespaces



