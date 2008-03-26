//==========================================================================
// ObTools::SSL: server.cc
//
// C++ wrapper for SSL server
//
// Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-ssl.h"
#include "ot-log.h"

namespace ObTools { namespace SSL {

//--------------------------------------------------------------------------
// Overridden factory for creating a client socket - returns an SSL socket
// and runs SSL accept on it
Net::TCPSocket *TCPServer::create_client_socket(int client_fd)
{
  OpenSSL::SSL *ssl = ctx?ctx->create_connection(client_fd):0;
  // Accept connection on SSL
  if (ssl)
  {
    int ret = OpenSSL::SSL_accept(ssl);
    if (ret < 1)
    {
      Context::log_errors("Failed to accept SSL");
      OpenSSL::SSL_free(ssl);
      return 0;
    }
  }

  return new SSL::TCPSocket(client_fd, ssl);
}

}} // namespaces



