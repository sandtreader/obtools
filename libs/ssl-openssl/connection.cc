//==========================================================================
// ObTools::SSL-OpenSSL: connection.cc
//
// C++ wrapper for OpenSSL connection
//
// Copyright (c) 2009 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-ssl-openssl.h"
#include "ot-log.h"

namespace ObTools { namespace SSL_OpenSSL {

//--------------------------------------------------------------------------
// Raw stream read wrapper
ssize_t Connection::cread(void *buf, size_t count)
{
  return SSL_read(ssl, buf, count);
}

//--------------------------------------------------------------------------
// Raw stream write wrapper
ssize_t Connection::cwrite(const void *buf, size_t count)
{
  return SSL_write(ssl, buf, count);
}

//--------------------------------------------------------------------------
// Get peer's X509 common name
string Connection::get_peer_cn()
{
  Crypto::Certificate cert(SSL_get_peer_certificate(ssl));
  return !cert?"":cert.get_cn();
}

//--------------------------------------------------------------------------
// Destructor
Connection::~Connection()
{
  SSL_shutdown(ssl);
  SSL_free(ssl);
}

}} // namespaces



