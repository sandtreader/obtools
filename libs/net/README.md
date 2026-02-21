# ObTools::Net

A TCP/UDP networking library for C++17 providing IP addresses, sockets, stream I/O, client connections, and multi-threaded servers with thread pooling.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Features

- **IP addressing**: IPv4 addresses with DNS lookup, CIDR netmasks, endpoints
- **TCP sockets**: stream read/write with safe error handling
- **UDP sockets**: datagram send/receive with multicast support
- **TCP client**: connection with timeout and local address binding
- **TCP servers**: single-threaded and multi-threaded (thread pool) variants
- **iostream integration**: `TCPStream` wraps sockets as standard C++ streams
- **Raw sockets**: for custom protocol implementations
- **Cross-platform**: Linux (primary), Windows (Winsock)

## Dependencies

- `ot-mt` - Multithreading (for server thread pools)
- `ext-wsock32`, `ext-iphlpapi` - Windows only

## Quick Start

```cpp
#include "ot-net.h"
using namespace ObTools;
```

### IP Addresses

```cpp
// Create from dotted quad or hostname
Net::IPAddress addr("192.168.1.1");
Net::IPAddress host("www.example.com");  // DNS lookup

// Access
string dq = addr.get_dotted_quad();   // "192.168.1.1"
string name = addr.get_hostname();     // reverse DNS
uint32_t raw = addr.hbo();            // host byte order

// Properties
if (addr.is_multicast()) { /* 224-239.x.x.x */ }
if (addr.is_broadcast()) { /* 255.255.255.255 */ }
if (addr) { /* valid address */ }

// Network masking
Net::IPAddress network = addr & Net::IPAddress("255.255.255.0");
```

### CIDR / Masked Addresses

```cpp
Net::MaskedAddress subnet("192.168.1.0/24");
int bits = subnet.get_network_bits();    // 24
string cidr = subnet.get_cidr();          // "192.168.1.0/24"

// Check if an address is in the subnet
Net::IPAddress test("192.168.1.42");
if (subnet == test) { /* address is in subnet */ }
```

### Endpoints

```cpp
// IP + port
Net::EndPoint ep("www.example.com", 80);
Net::EndPoint ep2("192.168.1.1:8080");   // from combined string

// Export to socket address
struct sockaddr_in sa;
ep.set(sa);

// String representation
string s = ep.str();  // "192.168.1.1:8080"
```

### TCP Client

```cpp
// Simple connection
Net::TCPClient client(Net::EndPoint("www.example.com", 80));
if (!client) { /* connection failed */ }

// With timeout (seconds)
Net::TCPClient client(Net::EndPoint("host", 80), 5);

// With local address binding
Net::TCPClient client(
  Net::EndPoint("0.0.0.0", 12345),     // local
  Net::EndPoint("host", 80),            // remote
  5                                      // timeout
);

// Read/write
client.write("GET / HTTP/1.0\r\n\r\n");
string response;
client.readall(response);

// Network byte order integers
client.write_nbo_int(42);
uint32_t n = client.read_nbo_int();

// String operators
client << "Hello\n";
string line;
client >> line;
```

### TCP Socket I/O

```cpp
Net::TCPSocket& sock = /* from server or client */;

// Write
sock.write("hello");
sock.write(buffer, length);

// Read available data
string data;
sock.read(data);

// Read exact amount
string exact;
sock.read(exact, 1024);     // exactly 1024 bytes into string

char buf[1024];
sock.read_exact(buf, 1024);  // exactly 1024 bytes into buffer

// Read until close
string all;
sock.readall(all);

// Non-blocking check
if (sock.wait_readable(5)) { /* data available within 5 seconds */ }
```

### iostream Wrapper

```cpp
Net::TCPClient client(Net::EndPoint("host", 80));
Net::TCPStream stream(client);

// Use as standard iostream
stream << "GET / HTTP/1.0\r\n\r\n";
stream.flush();

string line;
while (getline(stream, line))
  cout << line << endl;
```

### UDP Sockets

```cpp
// Server: bind to port
Net::UDPSocket server(5000);

// Receive datagram
char buf[1024];
Net::EndPoint sender;
ssize_t n = server.recvfrom(buf, sizeof(buf), 0, &sender);

// Client: connected
Net::UDPSocket client(Net::EndPoint("host", 5000));
client.send("hello", 5);

// Unconnected send
Net::UDPSocket sock;
sock.sendto("hello", 5, 0, Net::EndPoint("host", 5000));

// Broadcast
Net::UDPSocket bcast;
bcast.enable_broadcast();
bcast.sendto(data, len, 0, Net::EndPoint("255.255.255.255", 5000));

// Multicast
server.join_multicast(Net::IPAddress("239.1.2.3"));
// ... receive ...
server.leave_multicast(Net::IPAddress("239.1.2.3"));
```

### Multi-Threaded TCP Server

Subclass `TCPServer` and implement `process()`:

```cpp
class EchoServer: public Net::TCPServer
{
public:
  EchoServer(int port): TCPServer(port, 5, 2, 10) {}
  // backlog=5, min_spare=2, max_threads=10

  void process(Net::TCPSocket& s, Net::EndPoint client) override
  {
    cout << "Connection from " << client << endl;
    string data;
    while (s.read(data))
      s.write(data);  // echo back
  }
};

EchoServer server(8080);
server.run();  // blocks, handles connections with thread pool
```

With client filtering:

```cpp
bool verify(Net::EndPoint client) const override
{
  // Only allow local connections
  return client.host.get_dotted_quad().substr(0, 4) == "127.";
}
```

Run in background:

```cpp
EchoServer server(8080);
Net::TCPServerThread bg(server);  // runs server.run() in a thread
// ... do other work ...
server.shutdown();
```

### Single-Threaded TCP Server

For simple use cases:

```cpp
Net::TCPSingleServer server(8080);
while (true)
{
  Net::TCPSocket *client = server.wait(30);  // 30s timeout
  if (client)
  {
    string data;
    client->readall(data);
    client->write("OK");
    delete client;
  }
}
```

### Socket Options

```cpp
Net::TCPSocket& sock = /* ... */;

sock.set_timeout(30);          // 30 second read/write timeout
sock.enable_keepalive();       // TCP keepalives
sock.enable_reuse();           // SO_REUSEADDR
sock.go_nonblocking();         // non-blocking mode
sock.set_ttl(64);              // IP TTL
sock.set_priority(5);          // socket priority (0-7)
sock.set_tos(0x10);            // IP TOS field

// Multicast
sock.set_multicast_ttl(4);
sock.join_multicast(mcast_addr);
```

### MAC Addresses

```cpp
Net::TCPSocket sock;
string mac = sock.get_mac(Net::IPAddress("192.168.1.1"));  // ARP lookup
set<string> local_macs = sock.get_host_macs();              // all local MACs
```

### Hostname

```cpp
string hostname = Net::Host::get_hostname();
```

## API Reference

### Address Classes

| Class | Key Methods |
|-------|-------------|
| `IPAddress` | `hbo()`, `nbo()`, `get_dotted_quad()`, `get_hostname()`, `is_broadcast()`, `is_multicast()` |
| `MaskedAddress` | `get_network_bits()`, `get_cidr()`, `operator==(IPAddress)` for subnet match |
| `EndPoint` | `str()`, `set(sockaddr_in&)`, members: `host`, `port` |
| `Protocol` | `TCP`, `UDP`, `UNKNOWN` static constants |
| `Port` | members: `host`, `proto`, `port` |

### Socket Classes

| Class | Key Methods |
|-------|-------------|
| `Socket` | `bind()`, `close()`, `shutdown()`, `set_timeout()`, `wait_readable()`, `local()`, `remote()` |
| `TCPSocket` | `read()`, `write()`, `read_exact()`, `readall()`, `read_nbo_int()`, `write_nbo_int()` |
| `UDPSocket` | `recv()`, `send()`, `recvfrom()`, `sendto()`, `enable_broadcast()` |
| `RawSocket` | Same as PacketSocket with raw IP access |

### Client/Server Classes

| Class | Key Methods |
|-------|-------------|
| `TCPClient` | Constructors with endpoint/timeout/local binding, `get_server()` |
| `TCPSingleServer` | `wait(timeout)` returns connected socket |
| `TCPServer` | `run()`, `process()` (pure virtual), `verify()`, `shutdown()` |
| `TCPServerThread` | Background thread wrapper for TCPServer |
| `TCPStream` | iostream wrapper for TCPSocket |

## Build

```
NAME            = ot-net
TYPE            = lib
DEPENDS         = ot-mt
WINDOWS-DEPENDS = ext-wsock32 ext-iphlpapi
```

## License

Copyright (c) 2003 Paul Clark. MIT License.
