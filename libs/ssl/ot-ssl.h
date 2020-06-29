//==========================================================================
// ObTools::SSL: ot-ssl.h
//
// Public definitions for ObTools::SSL
// Abstract SSL interface - does nothing here.  Subclass Context to
// implement particularly SSL providers
//
// Copyright (c) 2009 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_SSL_H
#define __OBTOOLS_SSL_H

#include "ot-net.h"
#include "ot-xml.h"

namespace ObTools { namespace SSL {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Abstract SSL connection
class Connection
{
public:
  //------------------------------------------------------------------------
  // Constructor
  Connection() {}

  //------------------------------------------------------------------------
  // Raw stream read wrapper
  virtual ssize_t cread(void *buf, size_t count)=0;

  //------------------------------------------------------------------------
  // Raw stream write wrapper
  virtual ssize_t cwrite(const void *buf, size_t count)=0;

  //------------------------------------------------------------------------
  // Get peer's X509 common name
  virtual string get_peer_cn()=0;

  //------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Connection() {}
};

//==========================================================================
// SSL application context
// Abstract superclass containing all basic SSL operations
class Context
{
public:
  //------------------------------------------------------------------------
  // Constructor
  Context() {}

  //------------------------------------------------------------------------
  // Create a new SSL connection from the context and bind it to the given fd
  // and accept() it
  virtual Connection *accept_connection(int fd) = 0;

  //------------------------------------------------------------------------
  // Create a new SSL connection from the context and bind it to the given fd
  // and connect() it
  virtual Connection *connect_connection(int fd) = 0;

  //------------------------------------------------------------------------
  // Set the SNI hostname for the context
  virtual void set_sni_hostname(const string& host) = 0;

  //------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Context() {}

  //------------------------------------------------------------------------
  // Static:  Log SSL errors - only logs 'text' here
  static void log_errors(const string& text);
};

//==========================================================================
// SSL-over-TCP socket
class TCPSocket: public Net::TCPSocket
{
protected:
   Connection *ssl;  // SSL connection, or 0 if basic TCP

public:
  //------------------------------------------------------------------------
  // Default constructor, invalid socket
  TCPSocket(): Net::TCPSocket(Net::Socket::INVALID_FD), ssl(0) {}

  //------------------------------------------------------------------------
  // Explicit constructor from existing fd and SSL object
  TCPSocket(int _fd, Connection *_ssl = 0): Net::TCPSocket(_fd), ssl(_ssl) {}

  //------------------------------------------------------------------------
  // Raw stream read wrapper override
  ssize_t cread(void *buf, size_t count);

  //------------------------------------------------------------------------
  // Raw stream write wrapper override
  ssize_t cwrite(const void *buf, size_t count);

  //------------------------------------------------------------------------
  // Get peer's X509 common name
  string get_peer_cn();

  //------------------------------------------------------------------------
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
  //------------------------------------------------------------------------
  // Constructor
  TCPClient(Context *ctx, Net::EndPoint endpoint);

  //------------------------------------------------------------------------
  // Constructor with a timeout on connection (in seconds)
  TCPClient(Context *ctx, Net::EndPoint endpoint, int timeout);

  //------------------------------------------------------------------------
  // Constructor, binding specific local address/port
  // port can be zero if you only want to bind address
  TCPClient(Context *ctx, Net::EndPoint local, Net::EndPoint remote);

  //------------------------------------------------------------------------
  // Constructor, binding specific local address/port and with timeout
  // port can be zero if you only want to bind address
  TCPClient(Context *ctx, Net::EndPoint local, Net::EndPoint remote,
            int timeout);

  //------------------------------------------------------------------------
  // Constructor, binding specific local address/port and with timeout and TTL
  // port can be zero if you only want to bind address
  TCPClient(Context *ctx, Net::EndPoint local, Net::EndPoint remote,
            int timeout, int ttl);

  //------------------------------------------------------------------------
  // Constructor from existing fd
  TCPClient(Context *ctx, int fd, Net::EndPoint remote);

  //------------------------------------------------------------------------
  // Get server endpoint
  Net::EndPoint get_server() const { return server; }

  //------------------------------------------------------------------------
  // Test for badness
  bool operator!() const { return !connected; }
};

//==========================================================================
// SSL client details
struct ClientDetails
{
  Net::EndPoint address;   // IP address/port
  string cert_cn;          // CN from certificate, or empty if not provided
  string mac;              // Local MAC address, empty if not known

  ClientDetails() {}
  ClientDetails(const Net::EndPoint& _addr, const string& _cn="",
                const string& _mac=""):
    address(_addr), cert_cn(_cn), mac(_mac) {}
};

//--------------------------------------------------------------------------
// << operator to write ClientDetails to ostream (server.cc)
ostream& operator<<(ostream& s, const ClientDetails& cd);

//==========================================================================
// TCP server (multi-threaded, multiple clients at once)
// Still abstract, but intercepts inbound connections and attaches SSL to
// them
// If ctx is 0, behaves exactly like a standard server
class TCPServer: public Net::TCPServer
{
private:
  Context *ctx;     // Optional SSL context

  //------------------------------------------------------------------------
  // Virtual process method (see ot-net.h), but taking ClientDetails
  // Note: Not abstract here to allow override of just the non-SSL process(),
  //       as with a plain Net::TCPServer
  virtual void process(SSL::TCPSocket &/*s*/,
                       const ClientDetails& /* client */) {}

  //------------------------------------------------------------------------
  // Override of normal process method to call the above - can be overridden
  // again if you don't need the SSL parts
  virtual void process(Net::TCPSocket &s, Net::EndPoint client);

public:
  //------------------------------------------------------------------------
  // Constructor with just port (INADDR_ANY binding)
  TCPServer(Context *_ctx, int _port, int _backlog=5,
            int min_spare=1, int max_threads=10):
    Net::TCPServer(_port, _backlog, min_spare, max_threads),
    ctx(_ctx) {}

  //------------------------------------------------------------------------
  // Constructor with specified address (specific binding)
  TCPServer(Context *_ctx, Net::EndPoint _address, int _backlog=5,
            int min_spare=1, int max_threads=10):
    Net::TCPServer(_address, _backlog, min_spare, max_threads),
    ctx(_ctx) {}

  //------------------------------------------------------------------------
  // Override of factory for creating a client socket
  virtual Net::TCPSocket *create_client_socket(int client_fd);
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_SSL_H
















