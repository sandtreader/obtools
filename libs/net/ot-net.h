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

namespace ObTools { namespace Net { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// IP Address (address.cc)
// Designed to be upgradeable to IPv6
class IP_Address
{
private:
  uint32_t address;   // Host byte order
  static const uint32_t BADADDR = 0xffffffff;
 
public:
  // Basic constructors
  IP_Address(): address(BADADDR) {}
  IP_Address(uint32_t in): address(in) {}

  //--------------------------------------------------------------------------
  // Name lookup constructor
  IP_Address(const char *hostname);  // Resolves name

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
// << operator to write IP_Address to ostream
// e.g. cout << ip;
ostream& operator<<(ostream& s, const IP_Address& e);

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
 
  // Virtual destructor - usually we just close  
  virtual ~Socket();

public:
  //--------------------------------------------------------------------------
  // Test for badness
  bool operator!() const { return fd<0; }

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
class TCP_Socket: public Socket
{
public:
  //--------------------------------------------------------------------------
  // Constructor - allocates socket
  TCP_Socket();

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
  void TCP_Socket::readall(string& s) throw (SocketError);

  //--------------------------------------------------------------------------
  // Write the given string to the socket, blocking until finished
  // Throws SocketError on failure
  void write(const string &s) throw(SocketError);

  //--------------------------------------------------------------------------
  // Write the given C string to the socket, blocking until finished
  // Throws SocketError on failure
  void TCP_Socket::write(const char *p) throw(SocketError);

};

//--------------------------------------------------------------------------
// << operator to write strings to TCP_Sockets
// NOTE: Not a general stream operator!
// e.g. s << "HELO\n";
TCP_Socket& operator<<(TCP_Socket& s, const string& t);

//--------------------------------------------------------------------------
// >> operator to read strings from TCP_Sockets
// Return whether stream still open - hence not chainable
// NOTE: Not a general stream operator!
// e.g. while (s >> buf) cout << buf;
bool operator>>(TCP_Socket& s, string& t);

//==========================================================================
// UDP Socket (socket.cc)
class UDP_Socket: public Socket
{
public:
  //--------------------------------------------------------------------------
  // Constructor - allocates socket
  UDP_Socket();

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
		    IP_Address *address_p, int *port_p);

  //--------------------------------------------------------------------------
  // Raw datagram sendto wrapper
  int csendto(const void *msg, size_t len, int flags,
	      IP_Address address, int port);

  //--------------------------------------------------------------------------
  // Safe datagram recv wrapper
  // Throws SocketError on failure
  ssize_t UDP_Socket::recv(void *buf, size_t len, int flags=0) 
    throw (SocketError);

  //--------------------------------------------------------------------------
  // Safe datagram send wrapper
  // Throws SocketError on failure
  int UDP_Socket::send(const void *buf, size_t len, int flags=0)
    throw (SocketError);

  //--------------------------------------------------------------------------
  // Safe datagram recvfrom wrapper
  // If address_p and/or port_p are non-null, sets them to the source of the
  // datagram
  // Throws SocketError on failure
  ssize_t UDP_Socket::recvfrom(void *buf, size_t len, int flags,
			       IP_Address *address_p, int *port_p)
    throw (SocketError);

  //--------------------------------------------------------------------------
  // Safe datagram sendto wrapper
  // If address_p and/or port_p are non-null, sets them to the source of the
  // datagram
  // Throws SocketError on failure
  ssize_t UDP_Socket::sendto(const void *buf, size_t len, int flags,
			     IP_Address address, int port)
    throw (SocketError);
};

//==========================================================================
// TCP client
class TCP_Client: public TCP_Socket
{
  IP_Address server_addr;
  int server_port;
  bool connected;

public:
  //--------------------------------------------------------------------------
  // Constructor 
  TCP_Client(IP_Address addr, int port);

  //--------------------------------------------------------------------------
  // Test for badness
  bool operator!() const { return !connected; }
};

//==========================================================================
// TCP server (single client at a time; not very useful)
class TCP_Single_Server: public TCP_Socket
{
public:
  TCP_Single_Server(int port);
};

//==========================================================================
// TCP server (multi-threaded, multiple clients at once)
class TCP_Multi_Server: public TCP_Socket
{
public:
  TCP_Multi_Server(int port);
};


//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_NET_H



