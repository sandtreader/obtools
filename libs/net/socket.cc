//==========================================================================
// ObTools::Net: socket.cc
//
// C++ wrapper for BSD sockets
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-net.h"
#ifdef __WIN32__
#include <io.h>
#define SOCKCLOSE closesocket
#define SOCKIOCTL ioctlsocket
#define SOCKERRNO WSAGetLastError()
#define CHECKFD(fd) if (fd == INVALID_SOCKET) fd=-1;
typedef int socklen_t;
#else
#include <sys/ioctl.h>
#define SOCKCLOSE close
#define SOCKIOCTL ioctl
#define SOCKERRNO errno
#define CHECKFD(fd)
#endif

#include <errno.h>
#include <stdint.h>

#define SOCKET_BUFFER_SIZE 1024

namespace ObTools { namespace Net {

//==========================================================================
// Abstract Socket 

//--------------------------------------------------------------------------
// Default destructor - close
Socket::~Socket()
{
  close();
}

//--------------------------------------------------------------------------
// Close
void Socket::close()
{
  if (fd>=0)
  {
    ::SOCKCLOSE(fd);
    fd = -1;
  }
}

//--------------------------------------------------------------------------
// Go non-blocking
void Socket::go_nonblocking()
{
  unsigned long n = 1;
  SOCKIOCTL(fd, FIONBIO, &n);
}

//--------------------------------------------------------------------------
// Go blocking
void Socket::go_blocking()
{
  unsigned long n = 0;
  SOCKIOCTL(fd, FIONBIO, &n);
}

//--------------------------------------------------------------------------
// Bind to a local port (TCP or UDP servers)
// Whether successful
bool Socket::bind(int port)
{
  // Bind to local port, allow any remote address or port 
  struct sockaddr_in saddr;
  saddr.sin_family      = AF_INET;
  saddr.sin_addr.s_addr = INADDR_ANY;
  saddr.sin_port        = htons(port);

  return !::bind(fd, (struct sockaddr *)&saddr, sizeof(saddr));
}

//--------------------------------------------------------------------------
// Get local address
// Only works if socket is bound or connected.  
// Because of multihoming, IP address may only be available if connected 
// to a specific remote host
EndPoint Socket::local()
{
  struct sockaddr_in name;
  socklen_t namelen = sizeof(name);

  if (!getsockname(fd, (struct sockaddr *)&name, &namelen))
    return EndPoint(name);
  else
    return EndPoint();
}

//--------------------------------------------------------------------------
// Get remote address
// Only works if socket is connected.  
EndPoint Socket::remote()
{
  struct sockaddr_in name;
  socklen_t namelen = sizeof(name);

  if (!getpeername(fd, (struct sockaddr *)&name, &namelen))
    return EndPoint(name);
  else
    return EndPoint();
}


//==========================================================================
// Socket exceptions

//------------------------------------------------------------------------
// Get error string
string SocketError::get_string()
{
  if (error)
    return string(strerror(error));
  else
    return string("EOF");
}

//------------------------------------------------------------------------
// << operator to write SocketError to ostream
// e.g. cout << e;
ostream& operator<<(ostream& s, const SocketError& e)
{ 
  if (e.error)
    s << "Socket Error (" << e.error << "): " << strerror(e.error); 
  else
    s << "Socket EOF";

  return s;
}

//==========================================================================
// TCP Socket 

//--------------------------------------------------------------------------
// Constructor - allocates socket
TCPSocket::TCPSocket()
{
  fd = socket(PF_INET, SOCK_STREAM, 0); 
  CHECKFD(fd)
}

//--------------------------------------------------------------------------
// Raw stream read wrapper
ssize_t TCPSocket::cread(void *buf, size_t count)
{ 
  ssize_t size;

#ifdef __WIN32__
  size = ::recv(fd, (char *)buf, count, 0);
  if (size == SOCKET_ERROR) size = -1;
#else
  // Silently loop on EINTR
  do
  {
    size = ::read(fd, buf, count); 
  }
  while (size<0 && errno == EINTR);
#endif

  return size;
}

//--------------------------------------------------------------------------
// Raw stream write wrapper
ssize_t TCPSocket::cwrite(const void *buf, size_t count)
{ 
  ssize_t size;

#ifdef __WIN32__
  size = ::send(fd, (const char *)buf, count, 0);
  if (size == SOCKET_ERROR) size = -1;
#else
  // Silently loop on EINTR
  do
  {
    size = ::write(fd, buf, count); 
  }
  while (size<0 && errno == EINTR);
#endif

  return size;
}

//--------------------------------------------------------------------------
// Safe stream read wrapper
// Throws SocketError on failure
ssize_t TCPSocket::read(void *buf, size_t count) throw (SocketError)
{ 
  ssize_t size = cread(buf, count);
  if (size < 0) throw SocketError(SOCKERRNO);
  return size;
}

//--------------------------------------------------------------------------
// Safe stream write wrapper
// Throws SocketError on failure
void TCPSocket::write(const void *buf, size_t count) throw (SocketError)
{ 
  ssize_t size = cwrite(buf, count);
  if (size!=(ssize_t)count) throw SocketError(SOCKERRNO);
}

//--------------------------------------------------------------------------
// Read data from the socket into a string
// Appends whatever read data is available to the given string 
// Returns whether successful (socket hasn't closed)
// Throws SocketError on failure
bool TCPSocket::read(string& s) throw (SocketError)
{
  char buf[SOCKET_BUFFER_SIZE+1];
  ssize_t size = read(buf, SOCKET_BUFFER_SIZE);
  if (size)
  {
    s.append(buf, size);
    return true;
  }
  else return false;
}

//--------------------------------------------------------------------------
// Read exact amount of data from the socket into a string
// Returns whether stream has closed (last size was 0)
// Throws SocketError on failure
bool TCPSocket::read(string& s, size_t count) throw (SocketError)
{
  char buf[SOCKET_BUFFER_SIZE+1];
  size_t done = 0;

  while (done < count)
  {
    // Limit read to buffer size, or what's wanted
    size_t n = SOCKET_BUFFER_SIZE;
    if (count-done < n) n = count-done;

    ssize_t size = read(buf, n);
    if (size)
    {
      s.append(buf, size);
      done += size;
    }
    else return false;
  }
  
  return true;
}

//--------------------------------------------------------------------------
// Read everything to stream close, blocking until finished
// Throws SocketError on failure
void TCPSocket::readall(string& s) throw (SocketError)
{
  while (read(s))
    ;
}
 
//--------------------------------------------------------------------------
// Write the given string to the socket, blocking until finished
// Throws SocketError on failure
void TCPSocket::write(const string &s) throw(SocketError)
{
  const char *p = s.data();
  write(p, s.size());
}

//--------------------------------------------------------------------------
// Write the given C string to the socket, blocking until finished
// Throws SocketError on failure
void TCPSocket::write(const char *p) throw(SocketError)
{
  write(p, strlen(p));
}

//--------------------------------------------------------------------------
// << operator to write strings to TCPSockets
// NOTE: Not a general stream operator!
// e.g. s << "HELO\n";
TCPSocket& operator<<(TCPSocket& s, const string& t)
{
  s.write(t);
  return s;
}

//--------------------------------------------------------------------------
// >> operator to read strings from TCPSockets
// Return whether stream still open - hence not chainable
// Erases string before appending
// NOTE: Not a general stream operator!
// e.g. while (s >> buf) cout << buf;
bool operator>>(TCPSocket& s, string& t)
{
  t.erase();
  return s.read(t);
}

//--------------------------------------------------------------------------
// Read a network byte order (MSB-first) 4-byte integer from the socket
// Throws SocketError on failure or EOF
uint32_t TCPSocket::read_nbo_int() throw (SocketError)
{
  uint32_t n;
  if (!read(&n, 4)) throw SocketError();
  return ntohl(n);
}

//--------------------------------------------------------------------------
// Ditto, but allowing the possibility of failure at EOF
// Throws SocketError on non-EOF failure
bool TCPSocket::read_nbo_int(uint32_t& n) throw (SocketError)
{
  if (!read(&n, 4)) return false;
  n=ntohl(n);
  return true;
}

//--------------------------------------------------------------------------
// Write a network byte order (MSB-first) 4-byte integer to the socket
// Throws SocketError on failure
void TCPSocket::write_nbo_int(uint32_t i) throw (SocketError)
{
  uint32_t n = htonl(i);
  write(&n, 4);
}

//==========================================================================
// UDP Socket 

//--------------------------------------------------------------------------
// Constructor - allocates socket
UDPSocket::UDPSocket()
{
  fd = socket(PF_INET, SOCK_DGRAM, 0); 
  CHECKFD(fd)
}

//--------------------------------------------------------------------------
// Constructor - allocates socket and binds to local port (UDP server)
UDPSocket::UDPSocket(int port)
{
  fd = socket(PF_INET, SOCK_DGRAM, 0); 
  CHECKFD(fd)
  if (!Socket::bind(port)) close();
}

//--------------------------------------------------------------------------
// Constructor - allocates socket and connects to remote port (UDP client)
// Use this to obtain local addressing for packets sent to this endpoint
UDPSocket::UDPSocket(EndPoint remote)
{
  fd = socket(PF_INET, SOCK_DGRAM, 0); 
  CHECKFD(fd)

  struct sockaddr_in saddr;
  remote.set(saddr);

  if (fd>=0 && connect(fd, (struct sockaddr *)&saddr, sizeof(saddr)))
    close();
}

//--------------------------------------------------------------------------
// Raw datagram recv wrapper
ssize_t UDPSocket::crecv(void *buf, size_t len, int flags)
{ 
  ssize_t size;

#ifdef __WIN32__
  size = ::recv(fd, (char *)buf, len, flags); 
  if (size == SOCKET_ERROR) size = -1;
#else
  // Silently loop on EINTR
  do
  {
    size = ::recv(fd, buf, len, flags); 
  }
  while (size<0 && errno == EINTR);
#endif

  return size;
}

//--------------------------------------------------------------------------
// Raw datagram send wrapper
int UDPSocket::csend(const void *msg, size_t len, int flags)
{ 
  int res;

#ifdef __WIN32__
  res = ::send(fd, (const char *)msg, len, flags); 
  if (res == SOCKET_ERROR) res = -1;
#else
  // Silently loop on EINTR
  do
  {
    res = ::send(fd, msg, len, flags); 
  }
  while (res<0 && errno == EINTR);
#endif

  return res;
}

//--------------------------------------------------------------------------
// Raw datagram recvfrom wrapper
// If endpoint_p is non-null, sets it to the source of the
// datagram
ssize_t UDPSocket::crecvfrom(void *buf, size_t len, int flags,
			     EndPoint *endpoint_p)
{
  struct sockaddr_in saddr;
  socklen_t slen = sizeof(saddr);
  ssize_t size;

#ifdef __WIN32__
  size = ::recvfrom(fd, (char *)buf, len, flags,
		    (struct sockaddr *)&saddr, &slen); 
  if (size == SOCKET_ERROR) size = -1;
#else
  // Silently loop on EINTR
  do
  {
    size = ::recvfrom(fd, buf, len, flags, 
		      (struct sockaddr *)&saddr, &slen); 
  }
  while (size<0 && errno == EINTR);
#endif

  if (size >= 0 && endpoint_p) *endpoint_p = EndPoint(saddr);
  
  return size;
}

//--------------------------------------------------------------------------
// Raw datagram sendto wrapper
int UDPSocket::csendto(const void *msg, size_t len, int flags,
		       EndPoint endpoint)
{
  struct sockaddr_in saddr;
  endpoint.set(saddr);
  int res;

#ifdef __WIN32__
  res = ::sendto(fd, (const char *)msg, len, flags,
		  (struct sockaddr *)&saddr, sizeof(saddr)); 
  if (res == SOCKET_ERROR) res = -1;
#else
  do
  {
    res = ::sendto(fd, msg, len, flags, 
		  (struct sockaddr *)&saddr, sizeof(saddr)); 
  }
  while (res<0 && errno == EINTR);
#endif

  return res;
};

//--------------------------------------------------------------------------
// Raw datagram sendmsg wrapper
int UDPSocket::csendmsg(struct iovec *gathers, int ngathers, int flags,
			EndPoint endpoint)
{
  struct sockaddr_in saddr;
  endpoint.set(saddr);
  int res;

#ifdef __WIN32__
#error No idea if this works in Windows
#else
  struct msghdr mh;
  mh.msg_name = &saddr;
  mh.msg_namelen = sizeof(saddr);
  mh.msg_iov = gathers;
  mh.msg_iovlen = ngathers;
  mh.msg_control = 0;
  mh.msg_controllen = 0;
  mh.msg_flags = 0;

  do
  {
    res = ::sendmsg(fd, &mh, flags); 
  }
  while (res<0 && errno == EINTR);
#endif

  return res;
};

//--------------------------------------------------------------------------
// Safe datagram recv wrapper
// Throws SocketError on failure
ssize_t UDPSocket::recv(void *buf, size_t len, int flags) throw (SocketError)
{
  ssize_t size = crecv(buf, len, flags);
  if (size < 0) throw SocketError(errno);
  return size;
}

//--------------------------------------------------------------------------
// Safe datagram send wrapper
// Throws SocketError on failure
int UDPSocket::send(const void *buf, size_t len, int flags) 
  throw (SocketError)
{
  int res = csend(buf, len, flags);
  if (res < 0) throw SocketError(errno);
  return res;
}

//--------------------------------------------------------------------------
// Safe datagram recvfrom wrapper
// If endpoint_p is non-null, sets them to the source of the datagram
// Throws SocketError on failure
ssize_t UDPSocket::recvfrom(void *buf, size_t len, int flags,
			    EndPoint *endpoint_p)
                               throw (SocketError)
{
  ssize_t size = crecvfrom(buf, len, flags, endpoint_p);
  if (size < 0) throw SocketError(errno);
  return size;
}

//--------------------------------------------------------------------------
// Safe datagram sendto wrapper
// Throws SocketError on failure
ssize_t UDPSocket::sendto(const void *buf, size_t len, int flags,
			  EndPoint endpoint)
                             throw (SocketError)
{
  int res = csendto(buf, len, flags, endpoint);
  if (res < 0) throw SocketError(errno);
  return res;
}

//--------------------------------------------------------------------------
// Safe datagram sendmsg wrapper
// Throws SocketError on failure
ssize_t UDPSocket::sendmsg(struct iovec *gathers, int ngathers, int flags,
			  EndPoint endpoint)
                             throw (SocketError)
{
  int res = csendmsg(gathers, ngathers, flags, endpoint);
  if (res < 0) throw SocketError(errno);
  return res;
}

}} // namespaces



