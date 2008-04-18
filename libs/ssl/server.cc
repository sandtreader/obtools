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
  OpenSSL *ssl = ctx?ctx->create_connection(client_fd):0;
  // Accept connection on SSL
  if (ssl)
  {
    int ret = SSL_accept(ssl);
    if (ret < 1)
    {
      Context::log_errors("Failed to accept SSL");
      SSL_free(ssl);
      return 0;
    }
  }

  return new SSL::TCPSocket(client_fd, ssl);
}

//--------------------------------------------------------------------------
// Override of normal process method to call the SSL version 
void TCPServer::process(Net::TCPSocket &s, Net::EndPoint client)
{
  // Downcast to our TCPSocket - should be safe because we create it above
  SSL::TCPSocket& ss = static_cast<SSL::TCPSocket&>(s);

  // Get Common Name from client certificate (if any)
  Crypto::Certificate cert = ss.get_peer_certificate();
  string cn;
  if (!!cert) cn = cert.get_cn();

  ClientDetails cd(client, cn);
  process(ss, cd);
}

//------------------------------------------------------------------------
// << operator to write ClientDetails to ostream
ostream& operator<<(ostream& s, const ClientDetails& cd)
{
  s << cd.address;
  if (!cd.cert_cn.empty()) s << " (" << cd.cert_cn << ")";
  return s;
}

}} // namespaces



