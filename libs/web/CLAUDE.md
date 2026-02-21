# CLAUDE.md - ObTools::Web Library

## Overview

`ObTools::Web` is an HTTP client/server library with URL handling, MIME headers, cookies, JWT, WebSocket, and caching. Lives under `namespace ObTools::Web`.

**Header:** `ot-web.h`
**Dependencies:** `ot-xml`, `ot-misc`, `ot-ssl`, `ot-crypto`, `ot-json`
**Platforms:** posix, web

## Key Classes

| Class | Purpose |
|-------|---------|
| `URL` | URL parsing, encoding/decoding, query parameters |
| `MIMEHeaders` | HTTP/MIME header read/write |
| `HTTPMessage` | HTTP request/response with headers and body |
| `HTTPClient` | HTTP/HTTPS client with persistence, progressive I/O, cookies, JWT |
| `HTTPServer` | Abstract multi-threaded HTTP server (extends `SSL::TCPServer`) |
| `SimpleHTTPServer` | Concrete server with URL handler registration |
| `URLHandler` | Abstract per-URL request handler |
| `Cookie` / `CookieJar` | Cookie parsing and storage |
| `JWT` | JSON Web Token parse/sign/verify |
| `WebSocketFrame` | WebSocket frame read/write |
| `WebSocketServer` | WebSocket message handling |
| `Cache` | HTTP cache with update checking |

## URL

```cpp
URL();  URL(const string& s);  URL(XML::Element& xml);

string get_scheme() const;   string get_host() const;
string get_path() const;     string get_query() const;
string get_fragment() const;
string get_text() const;     string str() const;

bool get_query(Misc::PropertyList& props) const;
string get_query_parameter(const string& name) const;
void set_query_parameter(const string& name, const string& value);

void append(const string& t);
void append(Misc::PropertyList& query);
URL resolve(const URL& base) const;
bool split(XML::Element& xml) const;

static string encode(const string& s, bool space_as_plus=true);
static string decode(const string& s, bool space_as_plus=true);
static string encode(const Misc::PropertyList& props, bool space_as_plus=true);
static void decode(const string& s, Misc::PropertyList& props, bool space_as_plus=true);
```

## MIMEHeaders

```cpp
MIMEHeaders();
void enable_folding(int max_line=60);

bool has(const string& name) const;
string get(const string& name) const;
void put(const string& name, const string& value);
void remove(const string& name);
void replace(const string& name, const string& value);
void put_date(const string& header="date");

list<string> get_all(const string& name) const;
list<string> get_all_splitting(const string& name, char delimiter=',') const;
static Misc::PropertyList split_parameters(string& value);

bool read(istream& in, bool append=false);
bool write(ostream& out) const;
static bool getline(istream& in, string& s);
```

## HTTPMessage

```cpp
// Members: method, url, code, reason, version, headers, body

HTTPMessage();
HTTPMessage(const string& method, const URL& url, const string& version="HTTP/1.0");
HTTPMessage(int code, const string& reason, const string& version="HTTP/1.0");

bool is_request();
bool read_headers(istream& in);
bool read(istream& in, bool read_to_eof=false);
bool write_headers(ostream& out) const;
bool write(ostream& out, bool headers_only=false) const;

void set_cookie(const string& name, const string& value,
                const string& path="", const string& domain="",
                Time::Stamp expires=Time::Stamp(), bool secure=false, bool http_only=false);
void get_cookies(map<string,string>& values) const;
string get_cookie(const string& name) const;
JSON::Value get_jwt_payload(const string& secret) const;
```

## HTTPClient

```cpp
HTTPClient(Net::EndPoint server, const string& ua="", int conn_timeout=0, int op_timeout=0);
HTTPClient(Net::EndPoint server, SSL::Context *ctx, const string& ua="", int conn_timeout=0, int op_timeout=0);
HTTPClient(const URL& url, SSL::Context *ctx=nullptr, const string& ua="", int conn_timeout=0, int op_timeout=0);

// Simple operations (return HTTP status code)
int get(const URL& url, string& body);
int del(const URL& url, string& body);
int post(const URL& url, const string& request_body, string& response_body);
int put(const URL& url, const string& content_type, istream& is, string& response_body);
int simple(const string& op, const URL& url, string& body);

// Full request/response
int do_fetch(HTTPMessage& request, HTTPMessage& response);
bool fetch(HTTPMessage& request, HTTPMessage& response);

// Persistence (HTTP/1.1 keep-alive)
void enable_persistence();
void close_persistence();

// Progressive download/upload
void enable_progressive();
void disable_progressive();
void enable_progressive_upload(unsigned long chunk_length);
unsigned long read(unsigned char *data, unsigned long length);
unsigned long write(unsigned char *data, unsigned long length);

// Cookies & JWT
void set_cookie_jar(CookieJar *jar);
void set_jwt(const JWT& jwt);
bool jwt_valid();

// WebSocket
int open_websocket(const URL& url, Net::TCPStream*& stream_p, const string& auth_header="");

void enable_keepalive();
```

## HTTPServer (abstract)

```cpp
HTTPServer(int port=80, const string& version="", int backlog=5,
           int min_spare=1, int max_threads=10, int timeout=90);
HTTPServer(SSL::Context *ctx, int port=80, ...);

void set_cors_origin(const string& pattern="*");
void add_response_header(const string& name, const string& value);
void enable_websocket();

// Override these:
virtual bool handle_request(const HTTPMessage& request, HTTPMessage& response,
                            const SSL::ClientDetails& client,
                            SSL::TCPSocket& socket, Net::TCPStream& stream) = 0;
virtual bool check_auth(HTTPMessage& request, HTTPMessage& response, const SSL::ClientDetails& client);
virtual void generate_progressive(...);
virtual void handle_websocket(...);
virtual void handle_close(...);
```

## SimpleHTTPServer

```cpp
SimpleHTTPServer(int port=80, ...);
SimpleHTTPServer(SSL::Context *ctx, int port=80, ...);
void add(URLHandler *h);
void remove(URLHandler *h);
```

## URLHandler (abstract)

```cpp
URLHandler(const string& url);
virtual bool handle_request(const HTTPMessage& request, HTTPMessage& response,
                            const SSL::ClientDetails& client) = 0;
struct Exception: public runtime_error { int code; };
```

## JWT

```cpp
JWT();
JWT(const string& text);             // parse
JWT(const JSON::Value& payload);     // create

bool operator!();                     // invalid?
bool verify(const string& secret);   // verify HMAC signature
void sign(const string& secret);     // sign with HMAC-SHA256
Time::Stamp get_expiry();
bool expired();
string str();                         // serialise
```

## Cookie / CookieJar

```cpp
// Cookie: name, value, expires, path, domain, http_only, secure
Cookie();  Cookie(const string& name, const string& value);
bool read_from(const string& header_value);
string str(bool attrs=false) const;

// CookieJar
CookieJar();
void take_cookies_from(const HTTPMessage& response, const URL& origin);
void add_cookies_to(HTTPMessage& request);
void prune(bool session_ended=false);
```

## WebSocket

```cpp
// WebSocketFrame
WebSocketFrame();
enum class Opcode { continuation=0, text=1, binary=2, close=8, ping=9, pong=10 };
bool read(Net::TCPStream& stream);
bool write(Net::TCPStream& stream) const;

// WebSocketServer
WebSocketServer(Net::TCPStream& stream);
bool read(string& msg);
bool write(const string& msg);
bool write_binary(string& msg);
void close();
```

## Cache

```cpp
Cache(const File::Directory& dir, SSL::Context *ssl_ctx=nullptr, const string& ua="");
bool fetch(const URL& url, File::Path& path, bool check_for_updates=false);
bool fetch(const URL& url, string& contents, bool check_for_updates=false);
bool set_update_interval(const URL& url, const string& interval);
void forget(const URL& url);
void update();
```

## File Layout

```
ot-web.h              - Public header
url.cc                - URL parsing/encoding
mime-headers.cc       - MIME headers
http-message.cc       - HTTPMessage
http-client.cc        - HTTPClient
http-server.cc        - HTTPServer, SimpleHTTPServer, URLHandler
cookies.cc            - Cookie, CookieJar
jwt.cc                - JWT
websocket.cc          - WebSocketFrame, WebSocketServer
cache.cc              - HTTP cache
test-url-gtest.cc     - URL tests
test-cookies.cc       - Cookie tests
test-http-client.cc   - HTTP client tests
test-jwt.cc           - JWT tests
```
