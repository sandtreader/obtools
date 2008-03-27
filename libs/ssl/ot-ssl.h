//==========================================================================
// ObTools::SSL: ot-ssl.h
//
// Public definitions for ObTools::SSL
// SSL/TLS Socket functions - wrapper around libssl
// 
// Copyright (c) 2008 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_SSL_H
#define __OBTOOLS_SSL_H

#include "ot-net.h"
#include "ot-crypto.h"

// This is rather ugly...  We want to use SSL as a namespace, but
// OpenSSL defines it as a struct.  Hence we redefine SSL here to 
// expand to OpenSSL for the duration of the OpenSSL header
#define SSL OpenSSL
#include <openssl/ssl.h>
#undef SSL

#include <openssl/err.h>

namespace ObTools { namespace SSL { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// SSL application context
class Context
{
  SSL_CTX *ctx;  // OpenSSL library context

public:
  //--------------------------------------------------------------------------
  // Constructor: Allocates context
  Context();

  //--------------------------------------------------------------------------
  // Use the given certificate
  void use_certificate(Crypto::Certificate& cert);

  //--------------------------------------------------------------------------
  // Use the given RSA private key
  void use_private_key(Crypto::RSAKey& rsa);

  //--------------------------------------------------------------------------
  // Create a new SSL connection from the context and bind it to the given fd
  OpenSSL *create_connection(int fd);

  //--------------------------------------------------------------------------
  // Destructor: Deallocates context
  ~Context();

  //--------------------------------------------------------------------------
  // Static:  Log SSL errors
  static void log_errors(const string& text);
};

//==========================================================================
// SSL-over-TCP socket
class TCPSocket: public Net::TCPSocket
{
protected:
  OpenSSL *ssl;  // SSL connection, or 0 if basic TCP

public:
  //--------------------------------------------------------------------------
  // Default constructor, invalid
  TCPSocket(): Net::TCPSocket(), ssl(0) {}

  //--------------------------------------------------------------------------
  // Explicit constructor from existing fd and SSL object
  TCPSocket(int _fd, OpenSSL *_ssl = 0): Net::TCPSocket(_fd), ssl(_ssl) {}

  //--------------------------------------------------------------------------
  // Raw stream read wrapper override
  ssize_t cread(void *buf, size_t count);

  //--------------------------------------------------------------------------
  // Raw stream write wrapper override
  ssize_t cwrite(const void *buf, size_t count);

  //--------------------------------------------------------------------------
  // Get peer's X509 certificate
  // Note, returned by value, will X509_free when done
  Crypto::Certificate get_peer_certificate();

  //--------------------------------------------------------------------------
  // Destructor
  virtual ~TCPSocket();
};

//==========================================================================
// TCP client - exact mimic of Net::TCPClient, but with Context added
// Context (ctx) can be set to 0 in all constructors to revert to simple TCP
class TCPClient: public TCPSocket
{
private:
  Net::EndPoint server;
  bool connected;

  void attach_ssl(Context *ctx);

public:
  //--------------------------------------------------------------------------
  // Constructor 
  TCPClient(Context *ctx, Net::EndPoint endpoint);

  //--------------------------------------------------------------------------
  // Constructor with a timeout on connection (in seconds)
  TCPClient(Context *ctx, Net::EndPoint endpoint, int timeout);

  //--------------------------------------------------------------------------
  // Constructor, binding specific local address/port
  // port can be zero if you only want to bind address
  TCPClient(Context *ctx, Net::EndPoint local, Net::EndPoint remote);

  //--------------------------------------------------------------------------
  // Constructor, binding specific local address/port and with timeout
  // port can be zero if you only want to bind address
  TCPClient(Context *ctx, Net::EndPoint local, Net::EndPoint remote, 
	    int timeout);

  //--------------------------------------------------------------------------
  // Constructor, binding specific local address/port and with timeout and TTL
  // port can be zero if you only want to bind address
  TCPClient(Context *ctx, Net::EndPoint local, Net::EndPoint remote, 
	    int timeout, int ttl);

  //--------------------------------------------------------------------------
  // Constructor from existing fd
  TCPClient(Context *ctx, int fd, Net::EndPoint remote);

  //--------------------------------------------------------------------------
  // Get server endpoint
  Net::EndPoint get_server() const { return server; }

  //--------------------------------------------------------------------------
  // Test for badness
  bool operator!() const { return !connected; }
};

//==========================================================================
// TCP server (multi-threaded, multiple clients at once)
// Still abstract, but intercepts inbound connections and attaches SSL to
// them
// If ctx is 0, behaves exactly like a standard server
class TCPServer: public Net::TCPServer
{
private:
  Context *ctx;     // Optional SSL context

public:
  //--------------------------------------------------------------------------
  // Constructor with just port (INADDR_ANY binding)
  TCPServer(Context *_ctx, int _port, int _backlog=5, 
	    int min_spare=1, int max_threads=10):
    Net::TCPServer(_port, _backlog, min_spare, max_threads),
    ctx(_ctx) {}

  //--------------------------------------------------------------------------
  // Constructor with specified address (specific binding)
  TCPServer(Context *_ctx, Net::EndPoint _address, int _backlog=5, 
	    int min_spare=1, int max_threads=10):
    Net::TCPServer(_address, _backlog, min_spare, max_threads),
    ctx(_ctx) {}

  //--------------------------------------------------------------------------
  // Override of factory for creating a client socket 
  virtual Net::TCPSocket *create_client_socket(int client_fd);
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_SSL_H
















