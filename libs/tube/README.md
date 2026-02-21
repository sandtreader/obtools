# ObTools::Tube

A binary message protocol library for C++17 providing asynchronous and synchronous (request-response) clients and servers over TCP/SSL. Messages use a Tag-Length-Value (TLV) format with 4-byte tags, thread-pooled servers, and session management.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Features

- **Async messaging**: fire-and-forget message sending with background I/O threads
- **Sync request-response**: correlated request/response with configurable timeout
- **Bidirectional**: servers can also initiate requests to connected clients
- **Tagged messages**: 4-byte tags for message routing (e.g. `'HELO'`, `'DATA'`)
- **SSL/TLS**: optional encryption on all connection types
- **Thread-pooled server**: configurable min spare / max threads
- **Client filtering**: allow-list by CIDR address
- **Auto variants**: servers/clients with built-in background threads

## Dependencies

- `ot-chan` - Binary channel protocol
- `ot-cache` - Caching
- `ot-log` - Logging
- `ot-ssl-openssl` - SSL/TLS support
- `ot-misc` - Miscellaneous utilities

## Quick Start

```cpp
#include "ot-tube.h"
using namespace ObTools;
```

### Messages

```cpp
// Create a message with a 4-byte tag
Tube::tag_t tag = Tube::string_to_tag("HELO");
Tube::Message msg(tag, "Hello, World!");

// Tag conversion
string tag_str = Tube::tag_to_string(tag);  // "HELO"
string stag = msg.stag();                    // "HELO"

// Validity
if (msg.is_valid()) { /* tag != 0 */ }
if (!msg) { /* invalid */ }
```

### Async Client

```cpp
// Connect to server
Tube::Client client(Net::EndPoint("server.example.com", 33380), "MyClient");
client.start();  // start background send/receive threads

// Send messages (non-blocking)
Tube::Message msg(Tube::string_to_tag("PING"), "data");
client.send(msg);

// Receive messages (blocking)
Tube::Message reply;
if (client.wait(reply))
  cout << "Got: " << reply.stag() << ": " << reply.data << endl;

// Non-blocking check
if (client.poll())
{
  Tube::Message m;
  client.wait(m);  // won't block since poll() was true
}

// Status
if (client.is_connected()) { /* connected */ }

client.shutdown();
```

### Async Client with SSL

```cpp
SSL_OpenSSL::Context ctx;
ctx.set_default_verify_paths();

Tube::Client client(Net::EndPoint("server", 33381), &ctx, "SecureClient");
client.start();
```

### Sync Client (Request-Response)

```cpp
Tube::SyncClient client(Net::EndPoint("server", 33380), 5, "SyncClient");
// timeout = 5 seconds
client.start();

Tube::Message request(Tube::string_to_tag("GETD"), "key=value");
Tube::Message response;

if (client.request(request, response))
  cout << "Response: " << response.data << endl;
else
  cout << "Timeout or error" << endl;
```

### Auto Sync Client

Handles async messages in background while also supporting sync requests:

```cpp
class MyClient: public Tube::AutoSyncClient
{
public:
  MyClient(Net::EndPoint server)
    : AutoSyncClient(server, 5, "MyClient") {}

  void handle_async_message(Tube::Message& msg) override
  {
    cout << "Async: " << msg.stag() << ": " << msg.data << endl;
  }
};

MyClient client(Net::EndPoint("server", 33380));
client.start();

// Sync requests work normally
Tube::Message req(Tube::string_to_tag("PING"));
Tube::Message resp;
client.request(req, resp);

// Async messages are dispatched to handle_async_message()
```

### Async Server

```cpp
class MyServer: public Tube::Server
{
public:
  MyServer(int port): Server(port, "MyServer") {}

  bool handle_message(const Tube::ClientMessage& msg) override
  {
    switch (msg.action)
    {
      case Tube::ClientMessage::STARTED:
        cout << "Client connected: " << msg.client.address << endl;
        break;

      case Tube::ClientMessage::MESSAGE_DATA:
        cout << "Message from " << msg.client.address
             << ": " << msg.msg.stag() << endl;

        // Send response back
        {
          Tube::ClientMessage reply(msg.client,
            Tube::string_to_tag("RPLY"), "response data");
          send(reply);
        }
        break;

      case Tube::ClientMessage::FINISHED:
        cout << "Client disconnected" << endl;
        break;

      default: break;
    }
    return true;
  }
};

MyServer server(33380);
server.open();
server.run();  // blocks
```

### Sync Server (Request-Response)

```cpp
class MySyncServer: public Tube::SyncServer
{
public:
  MySyncServer(int port): SyncServer(port, "MySyncServer") {}

  bool handle_request(const Tube::ClientMessage& request,
                      Tube::Message& response) override
  {
    if (request.msg.stag() == "PING")
    {
      response = Tube::Message(Tube::string_to_tag("PONG"), "alive");
      return true;
    }
    return false;  // unhandled
  }
};

MySyncServer server(33380);
server.open();
server.run();
```

### Auto Sync Server (Background Thread)

```cpp
class MyAutoServer: public Tube::AutoSyncServer
{
public:
  MyAutoServer(int port): AutoSyncServer(port, "AutoServer") {}

  bool handle_request(const Tube::ClientMessage& request,
                      Tube::Message& response) override
  {
    response = Tube::Message(Tube::string_to_tag("OK  "), "done");
    return true;
  }
};

MyAutoServer server(33380);
server.open();
// Server runs in its own thread - no need to call run()
// ... do other work ...
server.shutdown();
```

### Bidirectional Server

Server that can also initiate requests to connected clients:

```cpp
class MyBiServer: public Tube::BiSyncServer
{
public:
  MyBiServer(int port)
    : BiSyncServer(port, 5, "BiServer") {}  // 5s request timeout

  bool handle_request(const Tube::ClientMessage& request,
                      Tube::Message& response) override
  {
    // Handle client -> server requests
    response = Tube::Message(Tube::string_to_tag("OK  "));
    return true;
  }
};

MyBiServer server(33380);
server.open();

// Server -> client request (from another thread)
Tube::ClientMessage req(client_details, Tube::string_to_tag("PING"));
Tube::Message resp;
if (server.request(req, resp))
  cout << "Client replied: " << resp.data << endl;
```

### SSL Server

```cpp
SSL_OpenSSL::Context ctx;
ctx.use_certificate("/path/to/cert.pem");
ctx.use_private_key("/path/to/key.pem");

Tube::SyncServer server(&ctx, 33381, "SecureServer");
server.open();
server.run();
```

### Client Filtering

```cpp
Tube::Server server(33380);
server.allow(Net::MaskedAddress("192.168.1.0/24"));
server.allow(Net::MaskedAddress("10.0.0.0/8"));
server.open();
server.run();
// Only clients from allowed subnets can connect
```

## API Reference

### Message

| Member/Method | Description |
|---------------|-------------|
| `tag` | 4-byte message tag (0 = invalid) |
| `flags` | Protocol flags (sync request ID embedded) |
| `data` | Message payload string |
| `is_valid()` / `operator!()` | Validity check |
| `stag()` | Tag as human-readable string |

### Client Classes

| Class | Key Methods |
|-------|-------------|
| `Client` | `start()`, `send(msg)`, `wait(msg)`, `poll()`, `shutdown()` |
| `SyncClient` | Adds `request(req, resp)` with timeout |
| `AutoSyncClient` | Adds `handle_async_message(msg)` virtual |

### Server Classes

| Class | Key Methods |
|-------|-------------|
| `Server` | `open()`, `run()`, `send(msg)`, `allow(addr)`, `shutdown()` |
| `SyncServer` | Adds `handle_request(req, resp)` pure virtual |
| `AutoSyncServer` | Runs in background thread |
| `BiSyncServer` | Adds `request(req, resp)` for server-to-client |
| `AutoBiSyncServer` | Bidirectional + background thread |

## Build

```
NAME    = ot-tube
TYPE    = lib
DEPENDS = ot-chan ot-cache ot-log ot-ssl-openssl ot-misc
```

## License

Copyright (c) 2007 Paul Clark. MIT License.
