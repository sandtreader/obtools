//==========================================================================
// ObTools::Net: ot-net.h
//
// Public definitions for ObTools::Net
// C++ wrapping of BSD sockets etc.
// 
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_NET_H
#define __OBTOOLS_NET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdint.h>
#include <iostream>
#include <string>

#include "ot-mt.h"

namespace ObTools { namespace Net { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// IP Address (address.cc)
// Designed to be upgradeable to IPv6
class IPAddress
{
private:
  uint32_t address;   // Host byte order
  static const uint32_t BADADDR = 0xffffffff;
 
public:
  //--------------------------------------------------------------------------
  // Basic constructors
  IPAddress(): address(BADADDR) {}
  IPAddress(uint32_t in): address(in) {}

  //--------------------------------------------------------------------------
  // Name lookup constructors
  IPAddress(const string& hostname);

  //--------------------------------------------------------------------------
  // Get host byte order integer
  uint32_t hbo() const { return address; }

  //--------------------------------------------------------------------------
  // Get network byte order integer
  uint32_t nbo() const { return ::htonl(address); }

  //--------------------------------------------------------------------------
  // Get dotted quad
  string get_dotted_quad() const;

  //--------------------------------------------------------------------------
  // Get hostname (reverse lookup), or dotted quad
  string get_hostname() const;

  //--------------------------------------------------------------------------
  // Test for badness
  bool operator!() const { return address==BADADDR; }

  //--------------------------------------------------------------------------
  // == operator 
  bool operator==(const IPAddress& o) const { return address == o.address; }

  //--------------------------------------------------------------------------
  // != operator 
  bool operator!=(const IPAddress& o) const { return address != o.address; }

  //--------------------------------------------------------------------------
  // < operator to help with maps 
  bool operator<(const IPAddress& o) const { return address < o.address; }
};

//------------------------------------------------------------------------
// << operator to write IPAddress to ostream
// e.g. cout << ip;
ostream& operator<<(ostream& s, const IPAddress& ip);

//==========================================================================
// IP Protocol 
// Tries to be as near as possible an enum with extra
class Protocol
{
private:
  enum Proto
  {
    PROTO_UNKNOWN,
    PROTO_TCP,
    PROTO_UDP
  } proto;

public:
  //------------------------------------------------------------------------
  // Constructors
  Protocol(): proto(PROTO_UNKNOWN) {}
  Protocol(Proto _proto): proto(_proto) {}
  Protocol(const Protocol& p): proto(p.proto) {}
  Protocol(const string& ps);

  //------------------------------------------------------------------------
  //Comparison
  bool operator==(const Protocol& p) const { return p.proto == proto; }
  bool operator!=(const Protocol& p) const { return p.proto != proto; }
  bool operator<(const Protocol& p)  const { return p.proto  < proto; }

  //------------------------------------------------------------------------
  // ! operator to test for unknownness
  bool operator!() { return proto==PROTO_UNKNOWN; }

  //------------------------------------------------------------------------
  // Output to ostream
  void output(ostream& s) const;

  //------------------------------------------------------------------------
  // Standard protocols
  static Protocol UNKNOWN;
  static Protocol TCP;
  static Protocol UDP;
};

//------------------------------------------------------------------------
// << operator to write Protocol to ostream (address.cc)
// e.g. cout << proto; 
ostream& operator<<(ostream& s, const Protocol& p);

//==========================================================================
// Network endpoint (address.cc) - IP Address and port identifying client
class EndPoint
{
public: 
  IPAddress host;
  int port;              // Host Byte Order

  //--------------------------------------------------------------------------
  // Constructors
  EndPoint(): host(), port(0) {}
  EndPoint(IPAddress _host, int _port): host(_host), port(_port) {}
  EndPoint(uint32_t in, int _port): host(in), port(_port) {}

  // Constructor from a sockaddr_in - note ntohl/ntohs here!
  EndPoint(const struct sockaddr_in& saddr):
    host(ntohl(saddr.sin_addr.s_addr)), port(ntohs(saddr.sin_port)) {}

  //--------------------------------------------------------------------------
  // Export to sockaddr_in - note htonl/htons here!
  void set(struct sockaddr_in& saddr)
  {
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = host.nbo();
    saddr.sin_port = htons(port);
  }

  //--------------------------------------------------------------------------
  // Output to given stream
  void output(ostream& s) const;

  //--------------------------------------------------------------------------
  // Test for badness
  bool operator!() const { return !host; }

  //--------------------------------------------------------------------------
  // == operator 
  bool operator==(const EndPoint& o) const 
  { return host == o.host && port == o.port; }

  //--------------------------------------------------------------------------
  // != operator 
  bool operator!=(const EndPoint& o) const 
  { return host != o.host || port != o.port; }

  //--------------------------------------------------------------------------
  // < operator to help with maps 
  bool operator<(const EndPoint& o) const 
  { return host < o.host || (host == o.host && port < o.port); }
};

//------------------------------------------------------------------------
// << operator to write EndPoint to ostream
// e.g. cout << ep;
ostream& operator<<(ostream& s, const EndPoint& ep);

//==========================================================================
// Protocol endpoint (address.cc) - IP Address and port identifying client
// plus protocol
class Port
{
public: 
  IPAddress host;
  Protocol proto;
  int port;              // Host Byte Order

  //--------------------------------------------------------------------------
  // Constructors
  Port(): host(), port(0), proto(Protocol::TCP) {}
  Port(IPAddress _host, Protocol _proto, int _port): 
    host(_host), proto(_proto), port(_port) {}

  //--------------------------------------------------------------------------
  // Output to given stream
  void output(ostream& s) const;

  //--------------------------------------------------------------------------
  // Test for badness
  bool operator!() const { return !host; }

  //--------------------------------------------------------------------------
  // == operator 
  bool operator==(const Port& o) const 
  { return host == o.host && proto == o.proto && port == o.port; }

  //--------------------------------------------------------------------------
  // != operator 
  bool operator!=(const Port& o) const 
  { return host != o.host || proto != o.proto || port != o.port; }

  //--------------------------------------------------------------------------
  // < operator to help with maps 
  bool operator<(const Port& o) const 
  { return host < o.host 
      || (host == o.host && proto < o.proto) 
      || (host == o.host && proto == o.proto && port < o.port); }
};

//------------------------------------------------------------------------
// << operator to write Port to ostream
// e.g. cout << ep;
ostream& operator<<(ostream& s, const Port& p);

//==========================================================================
// Abstract Socket (socket.cc)
class Socket
{
protected:
  int fd;

  // Simple constructor - subclasses provide the fd
  Socket(int _fd): fd(_fd) {}

  // Default constructor - set bad initially
  Socket(): fd(-1) {}
 
  // Virtual destructor - default just closes
  virtual ~Socket();

public:
  //--------------------------------------------------------------------------
  // Test for badness
  bool operator!() const { return fd<0; }

  //--------------------------------------------------------------------------
  // Close
  void close();

  //--------------------------------------------------------------------------
  // Go non-blocking
  void go_nonblocking();

  //--------------------------------------------------------------------------
  // Go blocking (the default)
  void go_blocking();

  //--------------------------------------------------------------------------
  // Bind to a local port (TCP or UDP servers)
  // Whether successful
  bool bind(int port);
};

//==========================================================================
// Socket exceptions
class SocketError 
{
public:
  int error;  // errno value, or 0
  SocketError(int e=0): error(e) {}
};

//------------------------------------------------------------------------
// << operator to write SocketError to ostream
// e.g. cout << e;
ostream& operator<<(ostream& s, const SocketError& e);

//==========================================================================
// TCP Socket (socket.cc)
class TCPSocket: public Socket
{
public:
  //--------------------------------------------------------------------------
  // Default constructor - allocates socket
  TCPSocket();

  //--------------------------------------------------------------------------
  // Explicit constructor - for use when (e.g.) accept() created an fd already
  TCPSocket(int _fd): Socket(_fd) {}

  //--------------------------------------------------------------------------
  // Raw stream read wrapper
  // NOTE:  All wrappers silently handle EINTR
  ssize_t cread(void *buf, size_t count);

  //--------------------------------------------------------------------------
  // Raw stream write wrapper
  ssize_t cwrite(const void *buf, size_t count);

  //--------------------------------------------------------------------------
  // Safe stream read wrapper
  // Throws SocketError on failure
  ssize_t read(void *buf, size_t count) throw (SocketError);

  //--------------------------------------------------------------------------
  // Safe stream write wrapper
  // Throws SocketError on failure
  void write(const void *buf, size_t count) throw (SocketError);

  //--------------------------------------------------------------------------
  // Read data from the socket into a string
  // Appends whatever read data is available to the given string 
  // Returns whether successful (socket hasn't closed)
  // Throws SocketError on failure
  bool read(string& s) throw (SocketError);

  //--------------------------------------------------------------------------
  // Read exact amount of data from the socket into a string
  // Whether successful - all data was read before socket closed
  // Throws SocketError on failure
  bool read(string& s, size_t count) throw (SocketError);

  //--------------------------------------------------------------------------
  // Read everything to stream close, blocking until finished
  // Throws SocketError on failure
  void readall(string& s) throw (SocketError);

  //--------------------------------------------------------------------------
  // Write the given string to the socket, blocking until finished
  // Throws SocketError on failure
  void write(const string &s) throw(SocketError);

  //--------------------------------------------------------------------------
  // Write the given C string to the socket, blocking until finished
  // Throws SocketError on failure
  void write(const char *p) throw(SocketError);

  //--------------------------------------------------------------------------
  // Read a network byte order (MSB-first) 4-byte integer from the socket
  // Throws SocketError on failure or EOF
  uint32_t read_nbo_int() throw (SocketError);

  //--------------------------------------------------------------------------
  // Ditto, but allowing the possibility of failure at EOF
  // Throws SocketError on non-EOF failure
  bool read_nbo_int(uint32_t& n) throw (SocketError);

  //--------------------------------------------------------------------------
  // Write a network byte order (MSB-first) 4-byte integer to the socket
  // Throws SocketError on failure
  void write_nbo_int(uint32_t i) throw (SocketError);
};

//--------------------------------------------------------------------------
// << operator to write strings to TCPSockets
// NOTE: Not a general stream operator - use TCPStream below
// e.g. s << "HELO\n";
TCPSocket& operator<<(TCPSocket& s, const string& t);

//--------------------------------------------------------------------------
// >> operator to read strings from TCPSockets
// Return whether stream still open - hence not chainable
// Erases string before appending
// NOTE: Not a general stream operator - use TCPStream below
// e.g. while (s >> buf) cout << buf;
bool operator>>(TCPSocket& s, string& t);

//==========================================================================
// TCP Socket streams
class TCPStreamBuf: public streambuf
{
private:
  TCPSocket& s;
  int bufc;           // One-character buffer

protected:
  // Streambuf overflow - handles characters
  int overflow(int); 
   
  // Streambuf underflows - see STL manual for awful details
  int underflow();
  int uflow();

public:
  // Constructor
  TCPStreamBuf(TCPSocket& _s);
};

//TCP stream
class TCPStream: public iostream
{
public:
  // Constructor - like fstream
  TCPStream(TCPSocket& s): 
    iostream(new TCPStreamBuf(s)) {}

  // Destructor
  ~TCPStream() { delete rdbuf(); }
};

//==========================================================================
// UDP Socket (socket.cc)
class UDPSocket: public Socket
{
public:
  //--------------------------------------------------------------------------
  // Constructor - allocates socket
  UDPSocket();

  //--------------------------------------------------------------------------
  // Constructor - allocatebs socket and binds to local port (UDP server)
  UDPSocket(int port);

  //--------------------------------------------------------------------------
  // Raw datagram recv wrapper
  // NOTE:  All wrappers silently handle EINTR
  ssize_t crecv(void *buf, size_t len, int flags=0);

  //--------------------------------------------------------------------------
  // Raw datagram send wrapper
  int csend(const void *msg, size_t len, int flags=0);

  //--------------------------------------------------------------------------
  // Raw datagram recvfrom wrapper
  // If endpoint_p is non-null, sets it to the source of the datagram
  ssize_t crecvfrom(void *buf, size_t len, int flags, EndPoint *endpoint_p);

  //--------------------------------------------------------------------------
  // Raw datagram sendto wrapper
  int csendto(const void *msg, size_t len, int flags, EndPoint endpoint);

  //--------------------------------------------------------------------------
  // Safe datagram recv wrapper
  // Throws SocketError on failure
  ssize_t recv(void *buf, size_t len, int flags=0) 
    throw (SocketError);

  //--------------------------------------------------------------------------
  // Safe datagram send wrapper
  // Throws SocketError on failure
  int send(const void *buf, size_t len, int flags=0)
    throw (SocketError);

  //--------------------------------------------------------------------------
  // Safe datagram recvfrom wrapper
  // If endpoint_p is non-null, sets it to the source of the datagram
  // Throws SocketError on failure
  ssize_t recvfrom(void *buf, size_t len, int flags, EndPoint *endpoint_p)
    throw (SocketError);

  //--------------------------------------------------------------------------
  // Safe datagram sendto wrapper
  // Throws SocketError on failure
  ssize_t sendto(const void *buf, size_t len, int flags, EndPoint endpoint)
    throw (SocketError);
};

//==========================================================================
// TCP client
class TCPClient: public TCPSocket
{
  EndPoint server;
  bool connected;

public:
  //--------------------------------------------------------------------------
  // Constructor 
  TCPClient(EndPoint endpoint);

  //--------------------------------------------------------------------------
  // Test for badness
  bool operator!() const { return !connected; }
};

//==========================================================================
// TCP server (multi-threaded, multiple clients at once)
// This is an abstract class which should be subclassed to implement
// process()
class TCPServer;  //forward

class TCPWorkerThread: public MT::PoolThread
{
public:
  TCPServer *server;
  int client_fd;
  EndPoint client_ep;

  TCPWorkerThread(MT::PoolReplacer<TCPWorkerThread>& _rep):
    MT::PoolThread(_rep) {}
  virtual void run();
};

class TCPServer: public TCPSocket
{
private:
  int port;
  int backlog;
  MT::ThreadPool<TCPWorkerThread> threadpool;

public:
  //--------------------------------------------------------------------------
  // Constructor.  
  TCPServer::TCPServer(int _port, int _backlog=5, 
		       int min_spare=1, int max_threads=10):
    port(_port), backlog(_backlog), threadpool(min_spare, max_threads) {}

  //--------------------------------------------------------------------------
  // Run server
  // Doesn't return unless it all falls apart.
  void run();

  //--------------------------------------------------------------------------
  // Virtual function to process a single connection on the given socket.  
  // Called in its own thread, this use blocking IO to read and write the
  // socket, and should just return when the socket ends or when bored
  virtual void process(TCPSocket &s, EndPoint client)=0;
};

//==========================================================================
// TCPServer thread class
// Runs TCP server in the background
class TCPServerThread: public MT::Thread
{
  TCPServer& server;
  void run()
  {
    server.run();
  }

public:
  TCPServerThread(TCPServer &s): server(s) { start(); }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_NET_H



