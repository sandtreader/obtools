# CLAUDE.md - ObTools::Net Library

## Overview

`ObTools::Net` is a TCP/UDP networking library providing IP addresses, sockets, clients, and multi-threaded servers. Lives under `namespace ObTools::Net`.

**Header:** `ot-net.h`
**Dependencies:** `ot-mt` (+ `ext-wsock32 ext-iphlpapi` on Windows)

## Key Classes

| Class | Purpose |
|-------|---------|
| `IPAddress` | IPv4 address with name resolution |
| `MaskedAddress` | IP address + netmask (CIDR) |
| `Protocol` | Protocol identifier (TCP/UDP) |
| `EndPoint` | IP address + port |
| `Port` | IP address + protocol + port |
| `Socket` | Abstract socket base |
| `TCPSocket` | TCP stream socket |
| `UDPSocket` | UDP datagram socket |
| `RawSocket` | Raw IP socket |
| `TCPClient` | TCP client with connect timeout |
| `TCPSingleServer` | Single-threaded TCP server |
| `TCPServer` | Multi-threaded TCP server (thread pool) |
| `TCPStream` | iostream wrapper for TCPSocket |

## IPAddress

```cpp
IPAddress();                           // invalid
IPAddress(uint32_t in);                // host byte order
IPAddress(const string& hostname);     // DNS lookup
uint32_t hbo() const;                  // host byte order
uint32_t nbo() const;                  // network byte order
string get_dotted_quad() const;        // "192.168.1.1"
string get_hostname() const;           // reverse DNS or dotted quad
bool is_broadcast() const;
bool is_multicast() const;             // 224-239.x.x.x
operator bool() const;                 // valid?
operator!() const;                     // invalid?
operator&(mask), operator~()           // masking
operator==, operator!=, operator<      // comparison
```

## MaskedAddress

```cpp
MaskedAddress(const string& cidr);     // "192.168.1.0/24"
int get_network_bits() const;          // e.g. 24
string get_cidr() const;              // "192.168.1.0/24"
operator==(const IPAddress& o) const;  // match: does address fit in subnet?
```

## EndPoint

```cpp
EndPoint();
EndPoint(IPAddress host, int port);
EndPoint(const string& hostname, int port);
EndPoint(const string& combined);       // "host:port"
EndPoint(const struct sockaddr_in& sa); // from socket address
void set(struct sockaddr_in& sa);       // export to socket address
string str() const;
operator bool() const;                  // valid host?
operator==, operator!=, operator<
```

## Socket (base class)

```cpp
fd_t get_fd();  fd_t detach_fd();
void close();  void shutdown();  void finish();
void go_nonblocking();  void go_blocking();
void enable_keepalive();  void enable_reuse();
void set_ttl(int hops);  void set_multicast_ttl(int hops);
void set_timeout(int secs, int usecs=0);
void set_priority(int prio);  void set_tos(int tos);
bool join_multicast(IPAddress);  bool leave_multicast(IPAddress);
bool bind(int port);  bool bind(EndPoint);
bool wait_readable(int timeout);  bool wait_writeable(int timeout);
EndPoint local() const;  EndPoint remote() const;
string get_mac(IPAddress, const string& device="") const;
set<string> get_host_macs() const;
```

## TCPSocket

```cpp
TCPSocket();  TCPSocket(fd_t fd);
ssize_t read(void *buf, size_t count);       // safe read
void write(const void *buf, size_t count);   // safe write
bool read(string& s);                         // read available
bool read(string& s, size_t count);           // read exact into string
bool read_exact(void *buf, size_t count);     // read exact into buffer
void readall(string& s);                      // read until close
void write(const string& s);
void write(const char *p);
uint32_t read_nbo_int();                      // network byte order 4-byte int
void write_nbo_int(uint32_t i);
operator<<(string), operator>>(string)
```

## UDPSocket

```cpp
UDPSocket();                                        // unbound
UDPSocket(int port);                                // bind to port (server)
UDPSocket(EndPoint local, bool, bool reuse=false);  // bind to interface
UDPSocket(EndPoint remote);                         // connect to remote (client)
UDPSocket(EndPoint local, EndPoint remote);         // bind + connect
void enable_broadcast();
// Inherits PacketSocket: recv, send, recvfrom, sendto, sendmsg
```

## TCPClient

```cpp
TCPClient(EndPoint endpoint);
TCPClient(EndPoint endpoint, int timeout);
TCPClient(EndPoint local, EndPoint remote);
TCPClient(EndPoint local, EndPoint remote, int timeout);
TCPClient(EndPoint local, EndPoint remote, int timeout, int ttl);
TCPClient(int fd, EndPoint remote);
EndPoint get_server() const;
operator!() const;    // connection failed?
```

## TCPServer

Subclass and implement `process()`. Uses a thread pool.

```cpp
TCPServer(int port, int backlog=5, int min_spare=1, int max_threads=10);
TCPServer(EndPoint address, int backlog=5, int min_spare=1, int max_threads=10);
void run();                                               // run main loop (blocks)
virtual bool verify(EndPoint) const;                      // client filter (default: accept all)
virtual void process(TCPSocket& s, EndPoint client) = 0;  // handle connection
Socket::fd_t initiate(EndPoint addr, int timeout);        // outgoing -> incoming
void take_over(int fd, EndPoint remote);                  // adopt existing socket
virtual TCPSocket *create_client_socket(int fd);          // factory override
void shutdown();
```

## TCPSingleServer

```cpp
TCPSingleServer(int port, int backlog=5);
TCPSingleServer(EndPoint address, int backlog=5);
TCPSocket *wait(int timeout=0);   // listen for connection (0=infinite)
```

## TCPStream

```cpp
TCPStream(TCPSocket& s);   // wrap socket as iostream
```

## TCPServerThread

```cpp
TCPServerThread(TCPServer& s);   // run server in background thread
```

## Host

```cpp
static string Host::get_hostname();   // local hostname
```

## SocketError

```cpp
struct SocketError { int error; string get_string() const; };
```

## File Layout

```
ot-net.h              - Public header
address.cc            - IPAddress, Protocol, EndPoint
socket.cc             - Socket, TCPSocket, UDPSocket, RawSocket
client.cc             - TCPClient
server.cc             - TCPServer, TCPWorkerThread
single-server.cc      - TCPSingleServer
stream.cc             - TCPStream, TCPStreamBuf
host.cc               - Host utilities
test-mac.cc           - MAC address tests
test-server.cc        - Server tests
```
