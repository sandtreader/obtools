# CLAUDE.md - ObTools::SSL Library

## Overview

`ObTools::SSL` defines the abstract SSL/TLS interface and concrete SSL-aware TCP socket, client, and server classes. The actual SSL implementation is provided by `ssl-openssl`. Lives under `namespace ObTools::SSL`.

**Header:** `ot-ssl.h`
**Dependencies:** `ot-net`, `ot-xml`, `ot-log`

## Architecture

`SSL::Context` and `SSL::Connection` are abstract. Pass a concrete implementation (e.g. from `ssl-openssl`) into `TCPClient`/`TCPServer`. Passing `nullptr` for the context gives plain TCP (no SSL).

## Key Classes

| Class | Purpose |
|-------|---------|
| `Connection` | Abstract: raw SSL read/write/peer CN |
| `Context` | Abstract: create SSL connections, SNI |
| `TCPSocket` | SSL-aware TCP socket (extends `Net::TCPSocket`) |
| `TCPClient` | SSL-aware TCP client |
| `TCPServer` | SSL-aware multi-threaded TCP server |
| `ClientDetails` | Client endpoint + certificate CN + MAC |

## Connection (abstract)

```cpp
virtual ssize_t cread(void *buf, size_t count) = 0;
virtual ssize_t cwrite(const void *buf, size_t count) = 0;
virtual string get_peer_cn() = 0;
```

## Context (abstract)

```cpp
virtual Connection *accept_connection(int fd) = 0;
virtual Connection *connect_connection(int fd) = 0;
virtual void set_sni_hostname(const string& host) = 0;
static void log_errors(const string& text);
```

## TCPSocket

Extends `Net::TCPSocket`. Overrides `cread`/`cwrite` to delegate to SSL `Connection` when present.

```cpp
TCPSocket();
TCPSocket(int fd, Connection *ssl = nullptr);
string get_peer_cn();
```

## TCPClient

```cpp
TCPClient(Context *ctx, Net::EndPoint endpoint);
TCPClient(Context *ctx, Net::EndPoint endpoint, int timeout);
TCPClient(Context *ctx, Net::EndPoint local, Net::EndPoint remote);
TCPClient(Context *ctx, Net::EndPoint local, Net::EndPoint remote, int timeout);
TCPClient(Context *ctx, Net::EndPoint local, Net::EndPoint remote, int timeout, int ttl);
TCPClient(Context *ctx, int fd, Net::EndPoint remote);
Net::EndPoint get_server() const;
bool operator!() const;
```

## TCPServer

Extends `Net::TCPServer`. Adds SSL-aware `process()` with `ClientDetails`.

```cpp
TCPServer(Context *ctx, int port, int backlog=5, int min_spare=1, int max_threads=10);
TCPServer(Context *ctx, Net::EndPoint address, int backlog=5, int min_spare=1, int max_threads=10);
virtual void process(SSL::TCPSocket& s, const ClientDetails& client);
```

## ClientDetails

```cpp
struct ClientDetails {
  Net::EndPoint address;
  string cert_cn;    // CN from client certificate (empty if none)
  string mac;        // local MAC address (empty if unknown)
};
ostream& operator<<(ostream&, const ClientDetails&);
```

## File Layout

```
ot-ssl.h             - Public header
context.cc           - Context base implementation
socket.cc            - TCPSocket implementation
client.cc            - TCPClient implementation
server.cc            - TCPServer implementation
```
