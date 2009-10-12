//==========================================================================
// ObTools::SSL-OpenSSL: connection.cc
//
// C++ wrapper for OpenSSL connection
//
// Copyright (c) 2009 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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



