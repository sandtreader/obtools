# CLAUDE.md - ObTools::Tube Library

## Overview

`ObTools::Tube` is a binary message protocol library providing async and sync (request-response) clients and servers over TCP/SSL with tagged messages, thread pools, and session management. Lives under `namespace ObTools::Tube`.

**Header:** `ot-tube.h`
**Dependencies:** `ot-chan`, `ot-cache`, `ot-log`, `ot-ssl-openssl`, `ot-misc`

## Types

```cpp
typedef uint32_t tag_t;       // 4-byte message tag (e.g. 'HELO')
typedef uint32_t flags_t;     // flags including sync request ID
typedef unsigned short id_t;  // request ID
```

## Message

```cpp
struct Message {
  tag_t tag;          // 0 = invalid
  flags_t flags;
  string data;

  Message();
  Message(tag_t tag, const string& data="", flags_t flags=0);
  bool is_valid();
  bool operator!();
  string stag() const;   // tag as string
};
```

## Tag Utilities

```cpp
string tag_to_string(tag_t tag);        // uint32 -> "HELO"
tag_t string_to_tag(const string& str); // "HELO" -> uint32
```

## Client Hierarchy

| Class | Purpose |
|-------|---------|
| `Client` | Async: send/receive messages with background threads |
| `SyncClient` | Sync: request/response with timeout |
| `AutoSyncClient` | Sync + auto-dispatches async messages |

### Client (async)

```cpp
Client(const Net::EndPoint& server, const string& name="Tube", bool fail_on_no_conn=false);
Client(const Net::EndPoint& server, SSL::Context *ctx, const string& name="Tube", bool fail_on_no_conn=false);

void start();
bool is_alive();  bool is_connected();
bool send(Message& msg);
virtual bool wait(Message& msg);    // blocks
virtual bool poll();                 // non-blocking check
void set_max_send_queue(int q);
virtual void shutdown();
```

### SyncClient

```cpp
SyncClient(Net::EndPoint server, int timeout=5, const string& name="Tube");
SyncClient(Net::EndPoint server, SSL::Context *ctx, int timeout=5, const string& name="Tube");

bool request(Message& request, Message& response);   // sync request/response
```

### AutoSyncClient

```cpp
AutoSyncClient(Net::EndPoint server, int timeout=5, const string& name="Tube");
AutoSyncClient(Net::EndPoint server, SSL::Context *ctx, int timeout=5, const string& name="Tube");

virtual void handle_async_message(Message& msg);   // override for async
```

## Server Hierarchy

| Class | Purpose |
|-------|---------|
| `Server` | Abstract base: async message handling |
| `SyncServer` | Sync: handles requests, returns responses |
| `AutoSyncServer` | SyncServer with own run() thread |
| `BiSyncServer` | Bidirectional: server can also make requests to clients |
| `AutoBiSyncServer` | BiSyncServer with own run() thread |

### Server (abstract)

```cpp
Server(int port, const string& name="Tube", int backlog=5, int min_spare=1, int max_threads=10, int client_timeout=300);
Server(SSL::Context *ctx, int port, ...);  // SSL variant
Server(Net::EndPoint local, ...);          // bind to specific address
Server(SSL::Context *ctx, Net::EndPoint local, ...);

void open();  void shutdown();
bool is_alive() const;
void allow(Net::MaskedAddress addr);      // client filter
bool send(ClientMessage& msg);
void set_max_send_queue(int q);

virtual bool handle_message(const ClientMessage& msg) = 0;  // implement this
```

### SyncServer

```cpp
// Same constructor variants as Server
virtual bool handle_request(const ClientMessage& request, Message& response) = 0;
virtual bool handle_async_message(const ClientMessage& msg);
```

### BiSyncServer

```cpp
// Same constructors + request_timeout parameter
bool request(ClientMessage& request, Message& response);  // server -> client request
virtual bool handle_client_async_message(const ClientMessage& msg);
```

## ClientMessage

```cpp
struct ClientMessage {
  enum Action { STARTED, MESSAGE_DATA, FINISHED, SHUTDOWN };
  const SSL::ClientDetails client;
  Message msg;
  const Action action;
};
```

## ClientSession / SessionMap

```cpp
struct ClientSession {
  SSL::TCPSocket& socket;
  Net::EndPoint client;
  bool alive;
  MT::Queue<Message> send_q;
};

class SessionMap {
  void add(Net::EndPoint client, ClientSession *s);
  void remove(Net::EndPoint client);
};
```

## File Layout

```
ot-tube.h               - Public header
tag.cc                   - Tag conversion
message.cc               - Message implementation
client.cc                - Client (async)
sync-client.cc           - SyncClient
auto-sync-client.cc      - AutoSyncClient
server.cc                - Server
sync-server.cc           - SyncServer
bi-sync-server.cc        - BiSyncServer
sync-request.cc          - SyncRequestCache
test-tube.cc             - Tests (gtest)
```
