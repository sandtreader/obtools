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
  // Constructors
  IP_Address(): address(BADADDR) {}
  IP_Address(uint32_t in): address(in) {}
  IP_Address(const char *hostname);  // Resolves name

  // Get network byte order integer
  uint32_t nbo() const { return ::htonl(address); }

  // Get hostname (reverse lookup), or dotted quad
  string get_hostname() const;

  // Output dotted quad to given stream
  void output_dotted_quad(ostream& s) const;

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
  // Test for badness
  bool operator!() const { return fd<0; }

  // Go non-blocking/blocking (blocking is default)
  void go_nonblocking();
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

//==========================================================================
// TCP Socket (socket.cc)
class TCP_Socket: public Socket
{
public:
  TCP_Socket();

  // Raw stream read/write wrappers
  ssize_t cread(void *buf, size_t count);
  ssize_t cwrite(void *buf, size_t count);

  // Safe stream read/write wrappers
  ssize_t read(void *buf, size_t count) throw (SocketError);
  ssize_t write(void *buf, size_t count) throw (SocketError);
};

//==========================================================================
// UDP Socket (socket.cc)
class UDP_Socket: public Socket
{
public:
  UDP_Socket();

  // Raw datagram recv/send/recvfrom/sendto wrappers
  ssize_t crecv(void *buf, size_t len, int flags);

  int csend(void *msg, size_t len, int flags);

  ssize_t crecvfrom(void *buf, size_t len, int flags,
		    IP_Address *address_p, int *port_p);

  int csendto(void *msg, size_t len, int flags,
	      IP_Address address, int port);

  // Safe datagram recv/send/recvfrom/sendto wrappers
  ssize_t UDP_Socket::recv(void *buf, size_t len, int flags) 
    throw (SocketError);

  int UDP_Socket::send(void *buf, size_t len, int flags) throw (SocketError);

  ssize_t UDP_Socket::recvfrom(void *buf, size_t len, int flags,
			       IP_Address *address_p, int *port_p)
    throw (SocketError);

  ssize_t UDP_Socket::sendto(void *buf, size_t len, int flags,
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
  TCP_Client(IP_Address addr, int port);

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



