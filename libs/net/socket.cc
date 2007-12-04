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
typedef int socklen_t;
typedef const char *sockopt_t;
#else
#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <net/if.h>
#define SOCKCLOSE close
#define SOCKIOCTL ioctl
#define SOCKERRNO errno
typedef const void *sockopt_t;
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
// Graceful shutdown
void Socket::shutdown()
{
#if defined(__WIN32__)
  ::shutdown(fd, SD_BOTH);
#else
  ::shutdown(fd, SHUT_RDWR);
#endif
}

//--------------------------------------------------------------------------
// Finish sending, but keep read-side open to receive results (e.g. HTTP)
void Socket::finish()
{
#if defined(__WIN32__)
  ::shutdown(fd, SD_SEND);
#else
  ::shutdown(fd, SHUT_WR);
#endif
}

//--------------------------------------------------------------------------
// Close
void Socket::close()
{
  if (fd != INVALID_FD)
  {
    ::SOCKCLOSE(fd);
    fd = INVALID_FD;
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
// Turn on keepalives
void Socket::enable_keepalive()
{
  int one = 1;
  setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (sockopt_t)&one, sizeof(int));
}

//--------------------------------------------------------------------------
// Enable reuse
void Socket::enable_reuse()
{
  int one = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (sockopt_t)&one, sizeof(int));
}

//--------------------------------------------------------------------------
// Set socket TTL
void Socket::set_ttl(int hops)
{
  setsockopt(fd, IPPROTO_IP, IP_TTL, (sockopt_t)&hops, sizeof(int));
}

//--------------------------------------------------------------------------
// Set timeout (receive and send) in seconds
void Socket::set_timeout(int secs)
{
#ifdef __WIN32__
  // Windows has it as an integer milliseconds (!)
  int ms = secs*1000;
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&ms, sizeof(ms));
  setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&ms, sizeof(ms));
#else
  struct timeval tv;
  tv.tv_sec = secs;
  tv.tv_usec = 0;

  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (sockopt_t)&tv, 
	     sizeof(struct timeval));
  setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (sockopt_t)&tv, 
	     sizeof(struct timeval));
#endif
}

//--------------------------------------------------------------------------
// Set socket priority (0-7)
void Socket::set_priority(int prio)
{
#ifdef __WIN32__
#warning set_priority not implemented in Windows
#else
  unsigned long n = (unsigned long)prio;

  setsockopt(fd, SOL_SOCKET, SO_PRIORITY, &n, sizeof(unsigned long));
#endif
}

//--------------------------------------------------------------------------
// Set IP TOS field
void Socket::set_tos(int tos)
{
#ifdef __WIN32__
#warning set_tos not implemented in Windows
#else
  unsigned long n = (unsigned long)tos;

  setsockopt(fd, IPPROTO_IP, IP_TOS, &n, sizeof(unsigned long));
#endif
}

//--------------------------------------------------------------------------
// Bind to a local port (TCP or UDP servers), all local addresses
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
// Bind to a local port (TCP or UDP servers), specified local address
// Whether successful
bool Socket::bind(EndPoint address)
{
  // Bind to local port, allow any remote address or port 
  struct sockaddr_in saddr;
  address.set(saddr);

  return !::bind(fd, (struct sockaddr *)&saddr, sizeof(saddr));
}

//--------------------------------------------------------------------------
// Select for read on a socket
// Use to allow a timeout on read/accept on blocking sockets
// Returns whether socket is readable within the given timeout (seconds)
bool Socket::wait_readable(int timeout)
{
  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);
  struct timeval tv;
  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  return select(fd+1, &rfds, 0, 0, &tv) == 1;

}

//--------------------------------------------------------------------------
// Select for write on a socket
// Use to allow a timeout on connect/write on blocking sockets
// Returns whether socket is readable within the given timeout (seconds)
bool Socket::wait_writeable(int timeout)
{
  fd_set wfds;
  FD_ZERO(&wfds);
  FD_SET(fd, &wfds);
  struct timeval tv;
  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  return select(fd+1, 0, &wfds, 0, &tv) == 1;
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

//--------------------------------------------------------------------------
// Get MAC address from ARP for any address (upper case hex with colons)
// Device name (e.g. "eth0") can be specified - if not given, all interfaces
// are searched
// Returns empty string if it can't find it
string Socket::get_mac(IPAddress ip, const string& device_name)
{
#ifdef __WIN32__
#warning get_mac not implemented in Windows
  return "";
#else
  // Check if device specified - if not, lookup all Ethernet interfaces 
  // and recurse
  if (device_name.empty())
  {
    struct if_nameindex *ifs = if_nameindex();
    if (!ifs) return "";

    for(struct if_nameindex *ifp = ifs; ifp->if_name; ifp++)
    {
      string ifname = ifp->if_name;

      // Ignore lo and aliases
      if (ifname=="lo" || ifname.find(':') != string::npos) continue;

      // Recurse with this interface, return if it succeeds
      string mac = get_mac(ip, ifname);
      if (!mac.empty()) return mac;
    }

    if_freenameindex(ifs);

    // No interfaces found it
    return "";
  }

  // Normal behaviour with device name given
  struct arpreq arp;
  memset(&arp, 0, sizeof(arp));
  struct sockaddr_in *sin = (struct sockaddr_in *)&arp.arp_pa;
  sin->sin_family = AF_INET;
  sin->sin_addr.s_addr = ip.nbo();
  strncpy(arp.arp_dev, device_name.c_str(), sizeof(arp.arp_dev));

  if (SOCKIOCTL(fd, SIOCGARP, &arp)<0) return "";

  // Check its complete
  if (!(arp.arp_flags & ATF_COM)) return "";

  // Create upper-case hex string
  char mac[18];
  unsigned char *p = (unsigned char *)arp.arp_ha.sa_data;
  sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
	  p[0], p[1], p[2], p[3], p[4], p[5]);
  return mac;
#endif
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
}

//--------------------------------------------------------------------------
// Raw stream read wrapper
ssize_t TCPSocket::cread(void *buf, size_t count)
{ 
  if (fd == INVALID_FD) return -1;

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
  while (fd != INVALID_FD && size<0 && errno == EINTR);
#endif

  return size;
}

//--------------------------------------------------------------------------
// Raw stream write wrapper
ssize_t TCPSocket::cwrite(const void *buf, size_t count)
{ 
  if (fd == INVALID_FD) return -1;

  ssize_t size;

#ifdef __WIN32__
  size = ::send(fd, (const char *)buf, count, 0);
  if (size == SOCKET_ERROR) size = -1;
#else
  // Silently loop on EINTR
  do
  {
    size = ::send(fd, buf, count, MSG_NOSIGNAL); 
  }
  while (fd != INVALID_FD && size<0 && errno == EINTR);
#endif

  return size;
}

//--------------------------------------------------------------------------
// Safe stream read wrapper
// Returns amount actually read - not necessarily all required!
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
// Returns whether data was all read, or stream closed (last size was zero)
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
// Read exact amount of data from the socket into a buffer
// Returns whether data was all read, or stream closed (last size was zero)
// Throws SocketError on failure
bool TCPSocket::read_exact(void *buf, size_t count) throw (SocketError)
{
  size_t done = 0;
  unsigned char *cbuf = (unsigned char *)buf;

  while (done < count)
  {
    ssize_t size = read(cbuf+done, count-done);
    if (size)
      done += size;
    else 
      return false;
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
  if (!read_exact(&n, 4)) throw SocketError();
  return ntohl(n);
}

//--------------------------------------------------------------------------
// Ditto, but allowing the possibility of failure at EOF
// Throws SocketError on non-EOF failure
bool TCPSocket::read_nbo_int(uint32_t& n) throw (SocketError)
{
  if (!read_exact(&n, 4)) return false;
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
}

//--------------------------------------------------------------------------
// Constructor - allocates socket and binds to local port (UDP server)
UDPSocket::UDPSocket(int port)
{
  fd = socket(PF_INET, SOCK_DGRAM, 0); 
  if (!Socket::bind(port)) close();
}

//--------------------------------------------------------------------------
// Constructor - allocates socket and connects to remote port (UDP client)
// Use this to obtain local addressing for packets sent to this endpoint
UDPSocket::UDPSocket(EndPoint remote)
{
  fd = socket(PF_INET, SOCK_DGRAM, 0); 

  struct sockaddr_in saddr;
  remote.set(saddr);

  if (fd!=INVALID_FD && connect(fd, (struct sockaddr *)&saddr, sizeof(saddr)))
    close();
}

//--------------------------------------------------------------------------
// Enable broadcast on this socket
void UDPSocket::enable_broadcast()
{
  unsigned long n = 1;

  setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (sockopt_t)&n, 
	     sizeof(unsigned long));
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
#warning No idea if this works in Windows
  return -99;
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



