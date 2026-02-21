# ObTools::Web

A comprehensive HTTP client/server library for C++17 with URL handling, MIME headers, cookies, JWT authentication, WebSocket support, and HTTP caching.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Features

- **URL handling**: parsing, encoding/decoding, query parameters, resolution
- **HTTP client**: GET/POST/PUT/DELETE, persistent connections, progressive I/O
- **HTTP server**: multi-threaded with thread pooling, URL handler routing
- **HTTPS**: transparent SSL/TLS via `SSL::Context`
- **MIME headers**: full read/write with folding support
- **Cookies**: client-side cookie jar with server-side Set-Cookie
- **JWT**: JSON Web Token parsing, signing (HMAC-SHA256), and verification
- **WebSocket**: client and server frame-level protocol handling
- **HTTP caching**: URL-based cache with configurable update intervals
- **CORS**: configurable cross-origin resource sharing

## Dependencies

- `ot-xml` - XML parsing
- `ot-misc` - PropertyList, UUID, etc.
- `ot-ssl` - SSL/TLS support
- `ot-crypto` - Cryptographic operations (HMAC for JWT)
- `ot-json` - JSON (for JWT payloads)

## Quick Start

```cpp
#include "ot-web.h"
using namespace ObTools;
```

### URL Handling

```cpp
// Parse URL
Web::URL url("https://example.com:8080/path?key=value#section");
string scheme = url.get_scheme();       // "https"
string host   = url.get_host();         // "example.com:8080"
string path   = url.get_path();         // "/path"
string query  = url.get_query();        // "key=value"
string frag   = url.get_fragment();     // "section"

// Query parameters
string val = url.get_query_parameter("key");  // "value"
url.set_query_parameter("page", "2");

// Get all query parameters
Misc::PropertyList params;
url.get_query(params);

// URL encoding/decoding
string encoded = Web::URL::encode("hello world");    // "hello+world"
string decoded = Web::URL::decode("hello+world");    // "hello world"

// Encode property list as query string
Misc::PropertyList props;
props.add("name", "Alice");
props.add("age", "30");
string qs = Web::URL::encode(props);  // "name=Alice&age=30"

// Resolve relative URL against base
Web::URL base("https://example.com/dir/page");
Web::URL relative("../other");
Web::URL resolved = relative.resolve(base);
```

### Simple HTTP Client

```cpp
// GET request
Web::HTTPClient client(Web::URL("https://api.example.com"));
string body;
int status = client.get(Web::URL("/data"), body);
if (status == 200)
  cout << body << endl;

// POST request
string response;
int status = client.post(Web::URL("/submit"), "{\"key\":\"value\"}", response);

// DELETE request
int status = client.del(Web::URL("/item/42"), body);
```

### HTTP Client with SSL

```cpp
SSL_OpenSSL::Context ctx;
ctx.set_default_verify_paths();

Web::HTTPClient client(
  Web::URL("https://api.example.com"),
  &ctx,              // SSL context
  "MyApp/1.0",       // User-Agent
  10,                // connection timeout (seconds)
  30                 // operation timeout (seconds)
);

string body;
client.get(Web::URL("/api/data"), body);
```

### Full HTTP Request/Response

```cpp
Web::HTTPMessage request("GET", Web::URL("/api/data"), "HTTP/1.1");
request.headers.put("accept", "application/json");
request.headers.put("authorization", "Bearer token123");

Web::HTTPMessage response;
if (client.fetch(request, response))
{
  cout << "Status: " << response.code << endl;
  cout << "Body: " << response.body << endl;
  string content_type = response.headers.get("content-type");
}
```

### Persistent Connections (Keep-Alive)

```cpp
Web::HTTPClient client(Web::URL("https://api.example.com"), &ctx);
client.enable_persistence();

// Multiple requests reuse the same connection
string body1, body2;
client.get(Web::URL("/endpoint1"), body1);
client.get(Web::URL("/endpoint2"), body2);

client.close_persistence();
```

### Progressive Download

```cpp
Web::HTTPClient client(Web::URL("https://files.example.com"), &ctx);
client.enable_progressive();

Web::HTTPMessage request("GET", Web::URL("/large-file"));
Web::HTTPMessage response;
client.do_fetch(request, response);

// Read in chunks
unsigned char buf[4096];
unsigned long n;
while ((n = client.read(buf, sizeof(buf))) > 0)
{
  // process chunk
}
```

### Cookies

```cpp
// Client-side cookie jar
Web::CookieJar jar;
Web::HTTPClient client(Web::URL("https://example.com"));
client.set_cookie_jar(&jar);

// Cookies are automatically sent/received
string body;
client.get(Web::URL("/login"), body);   // receives Set-Cookie
client.get(Web::URL("/profile"), body); // sends cookies back

// Server-side cookie setting
Web::HTTPMessage response(200, "OK");
response.set_cookie("session", "abc123",
                     "/",              // path
                     "example.com",    // domain
                     Time::Stamp(),    // expires (session cookie)
                     true,             // secure
                     true);            // http-only

// Read cookies from request
string session = request.get_cookie("session");
```

### JWT (JSON Web Tokens)

```cpp
// Create and sign
JSON::Value payload(JSON::Value::OBJECT);
payload.put("sub", "user123");
payload.put("exp", (int64_t)time(nullptr) + 3600);

Web::JWT jwt(payload);
jwt.sign("my-secret-key");
string token = jwt.str();  // "header.payload.signature"

// Parse and verify
Web::JWT received(token_string);
if (!received) { /* invalid structure */ }
if (!received.verify("my-secret-key")) { /* bad signature */ }
if (received.expired()) { /* token expired */ }

// Access payload
string user = received.payload["sub"].as_str();

// JWT in HTTP client
Web::HTTPClient client(Web::URL("https://api.example.com"), &ctx);
client.set_jwt(jwt);
// JWT is automatically added to requests as Bearer token
```

### HTTP Server

Subclass `HTTPServer` for full control:

```cpp
class MyServer: public Web::HTTPServer
{
public:
  MyServer(int port): HTTPServer(port, "MyServer/1.0") {}

  bool handle_request(const Web::HTTPMessage& request,
                      Web::HTTPMessage& response,
                      const SSL::ClientDetails& client,
                      SSL::TCPSocket& socket,
                      Net::TCPStream& stream) override
  {
    if (request.method == "GET" && request.url.get_path() == "/hello")
    {
      response.code = 200;
      response.reason = "OK";
      response.body = "{\"message\": \"Hello!\"}";
      response.headers.put("content-type", "application/json");
      return true;
    }

    response.code = 404;
    response.reason = "Not Found";
    return true;
  }
};

MyServer server(8080);
server.set_cors_origin("*");
server.run();
```

### SimpleHTTPServer with URL Handlers

```cpp
// Define handlers
class HelloHandler: public Web::URLHandler
{
public:
  HelloHandler(): URLHandler("/hello") {}

  bool handle_request(const Web::HTTPMessage& request,
                      Web::HTTPMessage& response,
                      const SSL::ClientDetails& client) override
  {
    response.code = 200;
    response.reason = "OK";
    response.body = "Hello, World!";
    return true;
  }
};

// Register handlers
Web::SimpleHTTPServer server(8080, "MyApp/1.0");
HelloHandler hello;
server.add(&hello);
server.run();
```

### HTTPS Server

```cpp
SSL_OpenSSL::Context ctx;
ctx.use_certificate("/path/to/cert.pem");
ctx.use_private_key("/path/to/key.pem");

Web::SimpleHTTPServer server(&ctx, 8443, "MyApp/1.0");
// ... add handlers ...
server.run();
```

### WebSocket Server

```cpp
class WsServer: public Web::HTTPServer
{
public:
  WsServer(int port): HTTPServer(port)
  {
    enable_websocket();
  }

  bool handle_request(const Web::HTTPMessage& request,
                      Web::HTTPMessage& response,
                      const SSL::ClientDetails& client,
                      SSL::TCPSocket& socket,
                      Net::TCPStream& stream) override
  {
    // handle normal HTTP...
    return true;
  }

  void handle_websocket(const Web::HTTPMessage& request,
                        const SSL::ClientDetails& client,
                        SSL::TCPSocket& socket,
                        Net::TCPStream& stream) override
  {
    Web::WebSocketServer ws(stream);
    string msg;
    while (ws.read(msg))
    {
      ws.write("Echo: " + msg);
    }
  }
};
```

### WebSocket Client

```cpp
Web::HTTPClient client(Web::URL("ws://example.com:8080"));
Net::TCPStream *stream = nullptr;
int status = client.open_websocket(Web::URL("/ws"), stream);
if (status == 101 && stream)
{
  // Send/receive frames
  Web::WebSocketFrame frame(Web::WebSocketFrame::Opcode::text);
  frame.payload = "Hello";
  frame.write(*stream);

  Web::WebSocketFrame reply;
  reply.read(*stream);
  cout << reply.payload << endl;
}
```

### MIME Headers

```cpp
Web::MIMEHeaders headers;
headers.put("content-type", "application/json");
headers.put("x-custom", "value");
headers.replace("content-type", "text/html");  // replace existing

string ct = headers.get("content-type");
bool has = headers.has("x-custom");
headers.remove("x-custom");

// Multi-value headers
list<string> values = headers.get_all("set-cookie");
list<string> parts = headers.get_all_splitting("accept", ',');

// Date header
headers.put_date();  // adds current date

// Read/write from streams
headers.read(input_stream);
headers.write(output_stream);
```

### HTTP Caching

```cpp
Web::Cache cache(File::Directory("/tmp/cache"), &ssl_ctx);

// Fetch with caching
string contents;
if (cache.fetch(Web::URL("https://example.com/data.json"), contents))
{
  // contents has the data (from cache or fresh)
}

// Fetch to file
File::Path path;
cache.fetch(Web::URL("https://example.com/image.png"), path);

// Set update interval
cache.set_update_interval(Web::URL("https://example.com/data.json"), "1h");

// Force update check
cache.update();

// Remove from cache
cache.forget(Web::URL("https://example.com/old.json"));
```

## API Reference

### URL

| Method | Returns | Description |
|--------|---------|-------------|
| `get_scheme/host/path/query/fragment()` | `string` | URL components |
| `get_query(props)` | `bool` | Parse query into PropertyList |
| `get_query_parameter(name)` | `string` | Single query parameter |
| `set_query_parameter(name, value)` | `void` | Set query parameter |
| `resolve(base)` | `URL` | Resolve relative to base |
| `encode(s)` / `decode(s)` | `string` | URL encode/decode (static) |

### HTTPClient

| Method | Returns | Description |
|--------|---------|-------------|
| `get(url, body)` | `int` | HTTP GET, returns status |
| `post(url, body, response)` | `int` | HTTP POST |
| `put(url, ct, is, response)` | `int` | HTTP PUT |
| `del(url, body)` | `int` | HTTP DELETE |
| `fetch(req, resp)` | `bool` | Full request/response |
| `enable_persistence()` | `void` | Enable keep-alive |
| `enable_progressive()` | `void` | Enable chunked download |
| `set_cookie_jar(jar)` | `void` | Attach cookie jar |
| `set_jwt(jwt)` | `void` | Set JWT for auth |
| `open_websocket(url, stream)` | `int` | Upgrade to WebSocket |

### HTTPServer / SimpleHTTPServer

| Method | Description |
|--------|-------------|
| `handle_request(req, resp, client, sock, stream)` | Pure virtual: handle HTTP request |
| `check_auth(req, resp, client)` | Virtual: auth check (default: allow) |
| `handle_websocket(req, client, sock, stream)` | Virtual: WebSocket handler |
| `set_cors_origin(pattern)` | Enable CORS |
| `add_response_header(name, value)` | Add default response header |
| `add(handler)` / `remove(handler)` | (SimpleHTTPServer) Register URL handlers |

### JWT

| Method | Returns | Description |
|--------|---------|-------------|
| `JWT(text)` | | Parse from token string |
| `JWT(payload)` | | Create from JSON payload |
| `verify(secret)` | `bool` | Verify HMAC signature |
| `sign(secret)` | `void` | Sign with HMAC-SHA256 |
| `expired()` | `bool` | Check exp claim |
| `str()` | `string` | Serialise to token string |

## Build

```
NAME      = ot-web
TYPE      = lib
DEPENDS   = ot-xml ot-misc ot-ssl ot-crypto ot-json
PLATFORMS = posix web
```

## License

Copyright (c) 2003 Paul Clark. MIT License.
