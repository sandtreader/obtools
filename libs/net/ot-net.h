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
 
  // Virtual destructor - usually we just close  
  virtual ~Socket();

public:
  // Raw read/write/recv wrappers
  ssize_t read(void *buf, size_t count)
  { return ::read(fd, buf, count); }

  ssize_t write(void *buf, size_t count)
  { return ::write(fd, buf, count); }

  ssize_t Socket::recv(void *buf, size_t len, int flags)
  { return ::recv(fd, buf, len, flags); }

  ssize_t Socket::recvfrom(void *buf, size_t len, int flags,
			   struct sockaddr *from, socklen_t *fromlen)
  { return ::recvfrom(fd, buf, len, flags, from, fromlen); }

};

//==========================================================================
// TCP Socket (socket.cc)
class TCP_Socket: Socket
{
protected:

public:
  TCP_Socket();
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_NET_H



