//==========================================================================
// ObTools::Net: ot-net.h
//
// Public definitions for ObTools::Net
// C++ wrapping of BSD sockets etc.
// 
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_NET_H
#define __OBTOOLS_NET_H

#include <stdint.h>
#include <iostream>
#include <string>
#include <string.h>

#if defined(__WIN32__)

// Windows headers & fixes
#include <winsock2.h>
typedef int socklen_t;

// Winsock initialisation
extern bool winsock_initialise();

#else

// Unix headers
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include "ot-mt.h"

namespace ObTools { namespace Net { 

//Make our lives easier without polluting anyone else
using namespace std;

//--------------------------------------------------------------------------
// INADDR_ANY address
#pragma GCC diagnostic ignored "-Wold-style-cast"
const uint32_t inaddr_any = INADDR_ANY;
#pragma GCC diagnostic pop

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
  uint32_t nbo() const { return htonl(address); }

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
  // Check for broadcast
  bool is_broadcast() const { return !address || address==BADADDR; }

  //--------------------------------------------------------------------------
  // Check for multicast (224-239.x.x.x)
  bool is_multicast() const 
  { return (address >> 28) == 0xE; }  // Top byte is 1110

  //--------------------------------------------------------------------------
  // & operator - get network number given a mask
  IPAddress operator&(const IPAddress& mask) const 
  { return IPAddress(address & mask.address); }

  //--------------------------------------------------------------------------
  // ~ operator - complement mask to get host mask
  IPAddress operator~() const { return IPAddress(~address); }

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
// Masked address - IP address and netmask
class MaskedAddress
{
public:
  IPAddress address;
  IPAddress mask;

  //--------------------------------------------------------------------------
  // Basic constructors
  MaskedAddress(): address(), mask() {}
  MaskedAddress(uint32_t _address, uint32_t _mask): 
    address(_address), mask(_mask) {}
  MaskedAddress(IPAddress _address, IPAddress _mask): 
    address(_address), mask(_mask) {}
  MaskedAddress(IPAddress _address):         // Host mask
    address(_address), mask() {}

  //--------------------------------------------------------------------------
  // Name lookup constructors - mask is dotted-quad
  MaskedAddress(const string& hostname, const string& mask_dq):
    address(hostname), mask(mask_dq) {}

  //--------------------------------------------------------------------------
  // Constructor from CIDR-form a.b.c.d/xx or a.b.c.d/A.B.C.D
  // - e.g. 192.168.1.0/24 or 192.168.1.0/255.255.255.0
  MaskedAddress(const string& cidr);

  //--------------------------------------------------------------------------
  // Get number of network bits in mask
  int get_network_bits() const;

  //--------------------------------------------------------------------------
  // Get CIDR form (/24 type)
  string get_cidr() const;

  //--------------------------------------------------------------------------
  // Comparators - same if both are the same
  // == operator 
  bool operator==(const MaskedAddress& o) const 
  { return address == o.address && mask == o.mask; }

  // != operator 
  bool operator!=(const MaskedAddress& o) const 
  { return address != o.address || mask != o.mask; }

  //--------------------------------------------------------------------------
  // Match operator - see if given address fits within masked subnet
  bool operator==(const IPAddress& o) const
  { return (address & mask) == (o & mask); }

};

//------------------------------------------------------------------------
// << operator to write MaskedAddress to ostream
// e.g. cout << addr;
// Always outputs in CIDR /N form
ostream& operator<<(ostream& s, const MaskedAddress& ip);

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
  EndPoint(const string& hostname, int _port): host(hostname), port(_port) {}

  // Constructor from a sockaddr_in - note ntohl/ntohs here!
  EndPoint(const struct sockaddr_in& saddr):
    host(ntohl(saddr.sin_addr.s_addr)), port(ntohs(saddr.sin_port)) {}

  //--------------------------------------------------------------------------
  // Export to sockaddr_in - note htonl/htons here!
  void set(struct sockaddr_in& saddr)
  {
    memset(&saddr, 0, sizeof(struct sockaddr_in)); // Stop valgrind whinging
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
  Port(): host(), proto(Protocol::TCP), port(0) {}
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
#if defined(__WIN32__)
  typedef SOCKET fd_t;
  static const fd_t INVALID_FD = INVALID_SOCKET;
#else
  typedef int fd_t;
  static const fd_t INVALID_FD = -1;
#endif
  fd_t fd;

  // Simple constructor - subclasses provide the fd
  Socket(int _fd): fd(_fd) {}

  // Default constructor - set bad initially
  Socket(): fd(INVALID_FD) {}

  // Virtual destructor - default just closes
  virtual ~Socket();

public:
  //--------------------------------------------------------------------------
  // Get fd
  fd_t get_fd() { return fd; }

  //--------------------------------------------------------------------------
  // Detach fd so it does not get auto-closed
  fd_t detach_fd() { fd_t t=fd; fd=INVALID_FD; return t; }

  //--------------------------------------------------------------------------
  // Test for badness
  bool operator!() const { return fd == INVALID_FD; }

  //--------------------------------------------------------------------------
  // Close
  void close();

  //--------------------------------------------------------------------------
  // Graceful shutdown
  void shutdown();

  //--------------------------------------------------------------------------
  // Finish sending, but keep read-side open to receive results (e.g. HTTP)
  void finish();

  //--------------------------------------------------------------------------
  // Go non-blocking
  void go_nonblocking();

  //--------------------------------------------------------------------------
  // Go blocking (the default)
  void go_blocking();

  //--------------------------------------------------------------------------
  // Turn on keepalives
  void enable_keepalive();

  //--------------------------------------------------------------------------
  // Enable reuse
  void enable_reuse();

  //--------------------------------------------------------------------------
  // Set socket TTL
  void set_ttl(int hops);

  //--------------------------------------------------------------------------
  // Set socket multicast TTL
  void set_multicast_ttl(int hops);

  //--------------------------------------------------------------------------
  // Set timeout (receive and send) in seconds and optional microseconds
  void set_timeout(int secs, int usecs = 0);

  //--------------------------------------------------------------------------
  // Set socket priority (0-7)
  void set_priority(int prio);

  //--------------------------------------------------------------------------
  // Set IP TOS field
  void set_tos(int tos);

  //--------------------------------------------------------------------------
  // Join a multicast group address (IP_ADD_MEMBERSHIP)
  bool join_multicast(IPAddress address);

  //--------------------------------------------------------------------------
  // Leave a multicast group address (IP_DROP_MEMBERSHIP)
  bool leave_multicast(IPAddress address);

  //--------------------------------------------------------------------------
  // Bind to a local port (TCP or UDP servers), all local addresses
  // Whether successful
  bool bind(int port);

  //--------------------------------------------------------------------------
  // Bind to a local port (TCP or UDP servers), specified local address
  // Whether successful
  bool bind(EndPoint address);

  //--------------------------------------------------------------------------
  // Select for read on a socket
  // Use to allow a timeout on read/accept on blocking sockets
  // Returns whether socket is readable within the given timeout (seconds)
  bool wait_readable(int timeout);

  //--------------------------------------------------------------------------
  // Select for write on a socket
  // Use to allow a timeout on connect/write on blocking sockets
  // Returns whether socket is readable within the given timeout (seconds)
  bool wait_writeable(int timeout);

  //--------------------------------------------------------------------------
  // Get local address
  // Only works if socket is bound or connected.  
  // Because of multihoming, IP address may only be available if connected 
  // to a specific remote host
  EndPoint local();

  //--------------------------------------------------------------------------
  // Get remote address
  // Only works if socket is connected.  
  EndPoint remote();

  //--------------------------------------------------------------------------
  // Get MAC address from ARP for any address (upper case hex with colons)
  // Device name (e.g. "eth0") can be specified - if not given, all interfaces
  // are searched
  // Returns empty string if it can't find it
  string get_mac(IPAddress ip, const string& device_name="");
};

//==========================================================================
// Socket exceptions
class SocketError 
{
public:
  int error;  // errno value, or 0
  SocketError(int e=0): error(e) {}

  // Get error string
  string get_string();
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
  // Overrideable in children - e.g. SSLSocket
  // NOTE:  All wrappers silently handle EINTR
  virtual ssize_t cread(void *buf, size_t count);

  //--------------------------------------------------------------------------
  // Raw stream write wrapper
  virtual ssize_t cwrite(const void *buf, size_t count);

  //--------------------------------------------------------------------------
  // Safe stream read wrapper
  // Returns amount actually read - not necessarily all required!
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
  // Read exact amount of data from the socket into a buffer
  // Returns whether data was all read, or stream closed (last size was zero)
  // Throws SocketError on failure
  bool read_exact(void *buf, size_t count) throw (SocketError);

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
  // Raw stream sendmsg wrapper
  int csendmsg(struct iovec *gathers, int ngathers, int flags=0);

  //--------------------------------------------------------------------------
  // Safe stream sendmsg wrapper
  // Throws SocketError on failure
  ssize_t sendmsg(struct iovec *gathers, int ngathers, int flags=0)
    throw (SocketError);

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
  int in_buf_size; 
  char *in_buf;       // Input buffer (may be zero for unbuffered)
  int out_buf_size;
  char *out_buf;      // Output buffer (may be zero for unbuffered)

protected:
  // Streambuf overflow - handles characters
  int overflow(int); 
  int sync();
   
  // Streambuf underflows - see STL manual for awful details
  int underflow();
  int uflow();
  streamsize showmanyc();

  // putback beyond buffer function - always fails
  int pbackfail(int) { return traits_type::eof(); }

public:
  // Standard buffer sizes
  static const int DEFAULT_IN_BUFFER = 1024;
  static const int DEFAULT_OUT_BUFFER = 1024;

  //--------------------------------------------------------------------------
  // Constructor
  TCPStreamBuf(TCPSocket& _s, 
	       int _in_buf_size = DEFAULT_IN_BUFFER,
	       int _out_buf_size = DEFAULT_OUT_BUFFER);

  //--------------------------------------------------------------------------
  // Destructor
  ~TCPStreamBuf();
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
  // Constructor - allocates socket and binds to local port (UDP server)
  UDPSocket(int port);

  //--------------------------------------------------------------------------
  // Constructor - allocates socket and binds to local specific interface
  // (UDP server) - note 'bool' not used, just to disambiguate with client
  // enable reuse - allow the socket to be re-used (e.g. for multicast
  //                listeners)
  UDPSocket(EndPoint local, bool, bool reuse = false);

  //--------------------------------------------------------------------------
  // Constructor - allocates socket and connects to remote port (UDP client)
  // Use this to obtain local addressing for packets sent to this endpoint
  UDPSocket(EndPoint remote);

  //--------------------------------------------------------------------------
  // Constructor - allocates socket and binds to local specific interface
  // and then connects to remote port (UDP client)
  UDPSocket(EndPoint local, EndPoint remote);

  //--------------------------------------------------------------------------
  // Enable broadcast on this socket
  void enable_broadcast();

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
  // Raw datagram sendmsg wrapper
  int csendmsg(struct iovec *gathers, int ngathers, int flags,
	       EndPoint endpoint);

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

  //--------------------------------------------------------------------------
  // Safe datagram sendmsg wrapper
  // Throws SocketError on failure
  ssize_t sendmsg(struct iovec *gathers, int ngathers, int flags,
		  EndPoint endpoint)
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
  // Constructor with a timeout on connection (in seconds)
  TCPClient(EndPoint endpoint, int timeout);

  //--------------------------------------------------------------------------
  // Constructor, binding specific local address/port
  // port can be zero if you only want to bind address
  TCPClient(EndPoint local, EndPoint remote);

  //--------------------------------------------------------------------------
  // Constructor, binding specific local address/port and with timeout
  // port can be zero if you only want to bind address
  TCPClient(EndPoint local, EndPoint remote, int timeout);

  //--------------------------------------------------------------------------
  // Constructor, binding specific local address/port and with timeout and TTL
  // port can be zero if you only want to bind address
  TCPClient(EndPoint local, EndPoint remote, int timeout, int ttl);

  //--------------------------------------------------------------------------
  // Constructor from existing fd
  TCPClient(int fd, EndPoint remote);

  //--------------------------------------------------------------------------
  // Get server endpoint
  EndPoint get_server() const { return server; }

  //--------------------------------------------------------------------------
  // Test for badness
  bool operator!() const { return !connected; }
};

//==========================================================================
// TCP basic server (single-threaded, only listens for a single connection
// at a time)
class TCPSingleServer: public TCPSocket
{
private:
  EndPoint address;
  int backlog;
  void start();
  
public:
  //--------------------------------------------------------------------------
  // Constructor with just port (INADDR_ANY binding)
  TCPSingleServer(int _port, int _backlog=5):
    TCPSocket(), address(IPAddress(inaddr_any), _port), backlog(_backlog)
    { start(); }

  //--------------------------------------------------------------------------
  // Constructor with specified address (specific binding)
  TCPSingleServer(EndPoint _address, int _backlog=5):
    TCPSocket(), address(_address), backlog(_backlog) 
    { start(); }

  //--------------------------------------------------------------------------
  // Listen for a connection and return a TCP socket
  // If timeout is non-zero, times out and returns 0 if no connection in 
  // that time
  // Returns connected socket or 0 if it fails
  TCPSocket *wait(int timeout=0);
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
    MT::PoolThread(_rep), server(0), client_fd(-1) {}
  virtual void run();
  virtual void die(bool wait=false);
};

class TCPServer: public TCPSocket
{
private:
  EndPoint address;
  int backlog;
  MT::ThreadPool<TCPWorkerThread> threadpool;
  bool alive;
  
  void start();

public:
  //--------------------------------------------------------------------------
  // Constructor with just port (INADDR_ANY binding)
  TCPServer(int _port, int _backlog=5, 
	    int min_spare=1, int max_threads=10):
    TCPSocket(), address(IPAddress(inaddr_any), _port), backlog(_backlog),
    threadpool(min_spare, max_threads), alive(true) { start(); }

  //--------------------------------------------------------------------------
  // Constructor with specified address (specific binding)
  TCPServer(EndPoint _address, int _backlog=5, 
	    int min_spare=1, int max_threads=10):
    TCPSocket(), address(_address), backlog(_backlog), 
    threadpool(min_spare, max_threads), alive(true) { start(); }

  //--------------------------------------------------------------------------
  // Run server
  // Doesn't return unless shutdown() called
  void run();

  //--------------------------------------------------------------------------
  // Virtual function to verify acceptability of a client before spawning
  // the worker thread.  This function operates in the dispatcher thread
  // and should be fast!
  // Defaults to allowing anything
  virtual bool verify(EndPoint) { return true; }

  //--------------------------------------------------------------------------
  // Virtual function to process a single connection on the given socket.  
  // Called in its own thread, this use blocking IO to read and write the
  // socket, and should just return when the socket ends or when bored
  virtual void process(TCPSocket &s, EndPoint client)=0;

  //--------------------------------------------------------------------------
  // Initiate an outgoing connection, and then treat it as if it was an
  // incoming one - mainly for P2P
  // Connection is run with a worker thread just like an incoming connection
  // Timeout is in seconds
  // Returns fd of connection
  Socket::fd_t initiate(EndPoint addr, int timeout);

  //--------------------------------------------------------------------------
  // Accept an existing socket into the server to be processed
  // Used for P2P where 'server' socket may be initiated at this end
  void take_over(int fd, Net::EndPoint remote_address);

  //--------------------------------------------------------------------------
  // Factory for creating a client socket - overridable in subclass 
  // (e.g. SSL::TCPServer)
  virtual TCPSocket *create_client_socket(int client_fd);

  //--------------------------------------------------------------------------
  // Shut down server
  void shutdown();

  //--------------------------------------------------------------------------
  // Destructor
  virtual ~TCPServer() { shutdown(); }
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



