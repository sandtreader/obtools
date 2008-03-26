//==========================================================================
// ObTools::SSL: socket.cc
//
// C++ wrapper for SSL sockets
//
// Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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

  return SSL_read(ssl, buf, count);
}

//--------------------------------------------------------------------------
// Raw stream write wrapper
ssize_t TCPSocket::cwrite(const void *buf, size_t count)
{ 
  // If not SSL, revert to basic
  if (!ssl) return Net::TCPSocket::cwrite(buf, count);

  return SSL_write(ssl, buf, count);
}

//--------------------------------------------------------------------------
// Get peer's X509 certificate
// Note, returned by value, will X509_free when done
Crypto::Certificate TCPSocket::get_peer_certificate()
{
  if (!ssl) return Crypto::Certificate();  // Invalid
  return Crypto::Certificate(OpenSSL::SSL_get_peer_certificate(ssl));
}

//--------------------------------------------------------------------------
// Destructor
TCPSocket::~TCPSocket()
{
  // Shutdown and free SSL connection
  if (ssl) 
  {
    OpenSSL::SSL_shutdown(ssl);
    OpenSSL::SSL_free(ssl);
  }
}

}} // namespaces


