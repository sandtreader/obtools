//==========================================================================
// ObTools::Net: socket.cc
//
// C++ wrapper for BSD sockets
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-net.h"
#include <sys/ioctl.h>
#include <errno.h>

namespace ObTools { namespace Net {

//==========================================================================
// Abstract Socket 

//--------------------------------------------------------------------------
// Default destructor - just close
Socket::~Socket()
{
  close(fd);
}

//--------------------------------------------------------------------------
// Go non-blocking
void Socket::go_nonblocking()
{
  unsigned long n = 1;
  ioctl(fd, FIONBIO, &n);
}

//--------------------------------------------------------------------------
// Go blocking
void Socket::go_blocking()
{
  unsigned long n = 0;
  ioctl(fd, FIONBIO, &n);
}

//==========================================================================
// TCP Socket 

//--------------------------------------------------------------------------
// Constructor - allocate socket
TCP_Socket::TCP_Socket()
{
  fd = socket(PF_INET, SOCK_STREAM, 0); 
}

//--------------------------------------------------------------------------
// Raw stream read wrapper
ssize_t TCP_Socket::cread(void *buf, size_t count)
{ 
  return ::read(fd, buf, count); 
}

//--------------------------------------------------------------------------
// Raw stream write wrapper
ssize_t TCP_Socket::cwrite(void *buf, size_t count)
{ 
  return ::write(fd, buf, count); 
}

//--------------------------------------------------------------------------
// Safe stream read wrapper
// Throws SocketError on failure
ssize_t TCP_Socket::read(void *buf, size_t count) throw (SocketError)
{ 
  ssize_t size = ::read(fd, buf, count);
  if (size < 0) throw SocketError(errno);
  return size;
}

//--------------------------------------------------------------------------
// Safe stream write wrapper
// Throws SocketError on failure
ssize_t TCP_Socket::write(void *buf, size_t count) throw (SocketError)
{ 
  ssize_t size = ::write(fd, buf, count);
  if (size < 0) throw SocketError(errno);
  return size;
}

//==========================================================================
// UDP Socket 

//--------------------------------------------------------------------------
// Constructor - allocate socket
UDP_Socket::UDP_Socket()
{
  fd = socket(PF_INET, SOCK_DGRAM, 0); 
}

//--------------------------------------------------------------------------
// Raw datagram recv wrapper
ssize_t UDP_Socket::crecv(void *buf, size_t len, int flags)
{ 
  return ::recv(fd, buf, len, flags); 
}

//--------------------------------------------------------------------------
// Raw datagram send wrapper
int UDP_Socket::csend(void *msg, size_t len, int flags)
{ 
  return ::send(fd, msg, len, flags); 
}

//--------------------------------------------------------------------------
// Raw datagram recvfrom wrapper
// If address_p and/or port_p are non-null, sets them to the source of the
// datagram
ssize_t UDP_Socket::crecvfrom(void *buf, size_t len, int flags,
			      IP_Address *address_p, int *port_p)
{
  struct sockaddr_in saddr;
  socklen_t slen = sizeof(saddr);

  ssize_t size = ::recvfrom(fd, buf, len, flags, 
			    (struct sockaddr *)&saddr, &slen); 

  if (size >= 0)
  {
    if (address_p) *address_p = IP_Address(ntohl(saddr.sin_addr.s_addr));
    if (*port_p)   *port_p    = ntohs(saddr.sin_port);
  }

  return size;
}

//--------------------------------------------------------------------------
// Raw datagram sendto wrapper
int UDP_Socket::csendto(void *msg, size_t len, int flags,
			IP_Address address, int port)
{
  struct sockaddr_in saddr;

  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = address.nbo();
  saddr.sin_port = htons(port);

  return ::sendto(fd, msg, len, flags, 
		  (struct sockaddr *)&saddr, sizeof(saddr)); 
};

//--------------------------------------------------------------------------
// Safe datagram recv wrapper
// Throws SocketError on failure
ssize_t UDP_Socket::recv(void *buf, size_t len, int flags) throw (SocketError)
{
  ssize_t size = ::recv(fd, buf, len, flags);
  if (size < 0) throw SocketError(errno);
  return size;
}

//--------------------------------------------------------------------------
// Safe datagram send wrapper
// Throws SocketError on failure
int UDP_Socket::send(void *buf, size_t len, int flags) throw (SocketError)
{
  int res = ::send(fd, buf, len, flags);
  if (res < 0) throw SocketError(errno);
  return res;
}

//--------------------------------------------------------------------------
// Safe datagram recvfrom wrapper
// If address_p and/or port_p are non-null, sets them to the source of the
// datagram
// Throws SocketError on failure
ssize_t UDP_Socket::recvfrom(void *buf, size_t len, int flags,
			     IP_Address *address_p, int *port_p)
                               throw (SocketError)
{
  ssize_t size = crecvfrom(buf, len, flags, address_p, port_p);
  if (size < 0) throw SocketError(errno);
  return size;
}

//--------------------------------------------------------------------------
// Safe datagram sendto wrapper
// If address_p and/or port_p are non-null, sets them to the source of the
// datagram
// Throws SocketError on failure
ssize_t UDP_Socket::sendto(void *buf, size_t len, int flags,
			   IP_Address address, int port)
                             throw (SocketError)
{
  int res = csendto(buf, len, flags, address, port);
  if (res < 0) throw SocketError(errno);
  return res;
}

}} // namespaces



