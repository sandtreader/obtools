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
  // Basic constructors
  IPAddress(): address(BADADDR) {}
  IPAddress(uint32_t in): address(in) {}

  //--------------------------------------------------------------------------
  // Name lookup constructor
  IPAddress(const char *hostname);  // Resolves name

  //--------------------------------------------------------------------------
  // Get network byte order integer
  uint32_t nbo() const { return ::htonl(address); }

  //--------------------------------------------------------------------------
  // Get hostname (reverse lookup), or dotted quad
  string get_hostname() const;

  //--------------------------------------------------------------------------
  // Output dotted quad to given stream
  void output_dotted_quad(ostream& s) const;

  //--------------------------------------------------------------------------
  // Test for badness
  bool operator!() const { return address==BADADDR; }
};

//------------------------------------------------------------------------
// << operator to write IPAddress to ostream
// e.g. cout << ip;
ostream& operator<<(ostream& s, const IPAddress& e);

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
};

//==========================================================================
// Socket exceptions
class SocketError 
{
public:
  int error;  // errno value
  SocketError(int e): error(e) {}
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
  // Returns whether stream has closed (last size was 0)
  // Throws SocketError on failure
  bool read(string& s) throw (SocketError);

  //--------------------------------------------------------------------------
  // Read everything to stream close, blocking until finished
  // Throws SocketError on failure
  void TCPSocket::readall(string& s) throw (SocketError);

  //--------------------------------------------------------------------------
  // Write the given string to the socket, blocking until finished
  // Throws SocketError on failure
  void write(const string &s) throw(SocketError);

  //--------------------------------------------------------------------------
  // Write the given C string to the socket, blocking until finished
  // Throws SocketError on failure
  void TCPSocket::write(const char *p) throw(SocketError);

};

//--------------------------------------------------------------------------
// << operator to write strings to TCPSockets
// NOTE: Not a general stream operator!
// e.g. s << "HELO\n";
TCPSocket& operator<<(TCPSocket& s, const string& t);

//--------------------------------------------------------------------------
// >> operator to read strings from TCPSockets
// Return whether stream still open - hence not chainable
// Erases string before appending
// NOTE: Not a general stream operator!
// e.g. while (s >> buf) cout << buf;
bool operator>>(TCPSocket& s, string& t);

//==========================================================================
// UDP Socket (socket.cc)
class UDPSocket: public Socket
{
public:
  //--------------------------------------------------------------------------
  // Constructor - allocates socket
  UDPSocket();

  //--------------------------------------------------------------------------
  // Raw datagram recv wrapper
  // NOTE:  All wrappers silently handle EINTR
  ssize_t crecv(void *buf, size_t len, int flags=0);

  //--------------------------------------------------------------------------
  // Raw datagram send wrapper
  int csend(const void *msg, size_t len, int flags=0);

  //--------------------------------------------------------------------------
  // Raw datagram recvfrom wrapper
  // If address_p and/or port_p are non-null, sets them to the source of the
  // datagram
  ssize_t crecvfrom(void *buf, size_t len, int flags,
		    IPAddress *address_p, int *port_p);

  //--------------------------------------------------------------------------
  // Raw datagram sendto wrapper
  int csendto(const void *msg, size_t len, int flags,
	      IPAddress address, int port);

  //--------------------------------------------------------------------------
  // Safe datagram recv wrapper
  // Throws SocketError on failure
  ssize_t UDPSocket::recv(void *buf, size_t len, int flags=0) 
    throw (SocketError);

  //--------------------------------------------------------------------------
  // Safe datagram send wrapper
  // Throws SocketError on failure
  int UDPSocket::send(const void *buf, size_t len, int flags=0)
    throw (SocketError);

  //--------------------------------------------------------------------------
  // Safe datagram recvfrom wrapper
  // If address_p and/or port_p are non-null, sets them to the source of the
  // datagram
  // Throws SocketError on failure
  ssize_t UDPSocket::recvfrom(void *buf, size_t len, int flags,
			       IPAddress *address_p, int *port_p)
    throw (SocketError);

  //--------------------------------------------------------------------------
  // Safe datagram sendto wrapper
  // If address_p and/or port_p are non-null, sets them to the source of the
  // datagram
  // Throws SocketError on failure
  ssize_t UDPSocket::sendto(const void *buf, size_t len, int flags,
			     IPAddress address, int port)
    throw (SocketError);
};

//==========================================================================
// TCP client
class TCPClient: public TCPSocket
{
  IPAddress server_addr;
  int server_port;
  bool connected;

public:
  //--------------------------------------------------------------------------
  // Constructor 
  TCPClient(IPAddress addr, int port);

  //--------------------------------------------------------------------------
  // Test for badness
  bool operator!() const { return !connected; }
};

//==========================================================================
// TCP server (multi-threaded, multiple clients at once)
// This is an abstract class which should be subclassed to implement
// process()
class TCPServer;  //forward

class TCPServerThread: public MT::PoolThread
{
public:
  TCPServer *server;
  int client_fd;
  IPAddress client_address;
  int client_port;

  TCPServerThread(MT::PoolReplacer<TCPServerThread>& _rep):
    MT::PoolThread(_rep) {}
  virtual void run();
};

class TCPServer: public TCPSocket
{
private:
  int port;
  int backlog;
  MT::ThreadPool<TCPServerThread> threadpool;

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
  virtual void process(TCPSocket &s, IPAddress client_address,
		       int client_port)=0;
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_NET_H



