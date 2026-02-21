# ObTools::SSL

Abstract SSL/TLS interface with SSL-aware TCP socket, client, and server classes. Designed as a strategy pattern: this library defines the abstractions, while `ssl-openssl` provides the concrete OpenSSL implementation.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Features

- **Abstract SSL interface**: pluggable SSL implementations via `Context` and `Connection`
- **SSL-aware TCP classes**: `TCPSocket`, `TCPClient`, `TCPServer` that transparently handle SSL
- **Optional SSL**: pass `nullptr` context for plain TCP - same API, no encryption
- **Client certificates**: access peer certificate CN via `ClientDetails`
- **Thread-pooled server**: extends `Net::TCPServer` with SSL handshake handling

## Dependencies

- `ot-net` - TCP/UDP networking
- `ot-xml` - XML parsing
- `ot-log` - Logging

## Quick Start

```cpp
#include "ot-ssl.h"
#include "ot-ssl-openssl.h"  // for concrete implementation
using namespace ObTools;
```

### SSL Client

```cpp
// Create SSL context (from ssl-openssl)
SSL_OpenSSL::Context ctx;

// Connect with SSL
SSL::TCPClient client(&ctx, Net::EndPoint("www.example.com", 443));
if (!client) { /* connection failed */ }

// Use like a normal TCPSocket
client.write("GET / HTTP/1.0\r\nHost: www.example.com\r\n\r\n");
string response;
client.readall(response);

// Access peer certificate
string cn = client.get_peer_cn();
```

### SSL Client with Timeout

```cpp
SSL_OpenSSL::Context ctx;
SSL::TCPClient client(&ctx, Net::EndPoint("host", 443), 10);  // 10s timeout
```

### Plain TCP (No SSL)

```cpp
// Pass nullptr for context - same API, no encryption
SSL::TCPClient client(nullptr, Net::EndPoint("host", 80));
client.write("GET / HTTP/1.0\r\n\r\n");
```

### SSL Server

```cpp
class MyServer: public SSL::TCPServer
{
public:
  MyServer(SSL::Context *ctx, int port)
    : TCPServer(ctx, port) {}

  void process(SSL::TCPSocket& s, const SSL::ClientDetails& client) override
  {
    cout << "Client: " << client.address << endl;
    if (!client.cert_cn.empty())
      cout << "Certificate CN: " << client.cert_cn << endl;

    string data;
    s.read(data);
    s.write("OK\n");
  }
};

SSL_OpenSSL::Context ctx;
// ... configure ctx with certificates ...
MyServer server(&ctx, 8443);
server.run();
```

### ClientDetails

The SSL server's `process()` receives a `ClientDetails` struct:

```cpp
void process(SSL::TCPSocket& s, const SSL::ClientDetails& client) override
{
  // Client IP address and port
  Net::EndPoint addr = client.address;

  // Certificate Common Name (empty if no client cert)
  string cn = client.cert_cn;

  // Local MAC address (empty if unknown)
  string mac = client.mac;

  cout << client << endl;  // stream output
}
```

## Architecture

```
┌─────────────────────────────────────────┐
│  Application Code                        │
│  (uses SSL::TCPClient, SSL::TCPServer)  │
└────────────┬────────────────────────────┘
             │
┌────────────┴────────────────────────────┐
│  ssl/ (this library)                     │
│  Abstract: Context, Connection           │
│  Concrete: TCPSocket, TCPClient,         │
│            TCPServer, ClientDetails      │
└────────────┬────────────────────────────┘
             │ delegates to
┌────────────┴────────────────────────────┐
│  ssl-openssl/                            │
│  Concrete: Context, Connection           │
│  (wraps OpenSSL SSL_CTX, SSL)           │
└──────────────────────────────────────────┘
```

## API Reference

### Abstract Classes

| Class | Methods |
|-------|---------|
| `Connection` | `cread(buf, count)`, `cwrite(buf, count)`, `get_peer_cn()` |
| `Context` | `accept_connection(fd)`, `connect_connection(fd)`, `set_sni_hostname(host)` |

### Concrete Classes

| Class | Key Methods |
|-------|-------------|
| `TCPSocket` | Inherits `Net::TCPSocket`, overrides `cread`/`cwrite` for SSL, adds `get_peer_cn()` |
| `TCPClient` | 6 constructor variants with context/endpoint/timeout/local/ttl, `get_server()` |
| `TCPServer` | `process(TCPSocket&, ClientDetails&)` virtual, `create_client_socket(fd)` |
| `ClientDetails` | Members: `address`, `cert_cn`, `mac` |

## Build

```
NAME    = ot-ssl
TYPE    = lib
DEPENDS = ot-net ot-xml ot-log
```

## License

Copyright (c) 2008 Paul Clark. MIT License.
