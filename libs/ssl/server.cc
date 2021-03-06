//==========================================================================
// ObTools::SSL: server.cc
//
// C++ wrapper for SSL server
//
// Copyright (c) 2008 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-ssl.h"
#include "ot-log.h"

#define SSL_ACCEPT_TIMEOUT 30

namespace ObTools { namespace SSL {

//--------------------------------------------------------------------------
// Overridden factory for creating a client socket - returns an SSL socket
// and runs SSL accept on it
Net::TCPSocket *TCPServer::create_client_socket(int client_fd)
{
  Connection *ssl = 0;

  if (ctx)
  {
    // Ensure a reasonable timeout is set while accepting
    Net::TCPSocket socket(client_fd);
    socket.set_timeout(SSL_ACCEPT_TIMEOUT);

    // Accept in SSL
    ssl = ctx->accept_connection(client_fd);

    // Reset to default timeout in case server doesn't want it
    socket.set_timeout(0);
    socket.detach_fd();  // Avoid close when socket goes out of scope!

    if (!ssl) return 0;
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
  string cn = ss.get_peer_cn();

  // Get MAC address from socket
  string mac = ss.get_mac(client.host);

  ClientDetails cd(client, cn, mac);
  process(ss, cd);
}

//--------------------------------------------------------------------------
// << operator to write ClientDetails to ostream
ostream& operator<<(ostream& s, const ClientDetails& cd)
{
  s << cd.address;
  if (!cd.cert_cn.empty()) s << " (" << cd.cert_cn << ")";
  if (!cd.mac.empty()) s << ", MAC " << cd.mac;
  return s;
}

}} // namespaces



