# CLAUDE.md - ObTools::SOAP Library

## Overview

`ObTools::SOAP` provides SOAP message creation, parsing, and HTTP transport for web services. Lives under `namespace ObTools::SOAP`.

**Header:** `ot-soap.h`
**Dependencies:** `ot-web`, `ot-msg`, `ot-init`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Parser` | XML parser with SOAP namespace handling |
| `Header` | SOAP header with role/mustUnderstand/relay |
| `Message` | SOAP envelope with headers and bodies |
| `Fault` | SOAP fault (error response) |
| `HTTPClient` | SOAP-over-HTTP client (extends `Web::HTTPClient`) |
| `URLHandler` | Abstract SOAP URL handler |
| `MessageInterface<CONTEXT>` | HTTP/HTTPS SOAP servers from `<soap>` config, feeding a `Message::Broker` |

## Message

```cpp
Message(const string& ns="");
void add_header(XML::Element *header, ...);
void add_body(XML::Element *body, ...);
XML::Element *get_body(const string& name) const;
list<XML::Element *> get_bodies() const;
list<Header> get_headers() const;
void flatten_bodies();   // resolve SOAP 1.1 href/id references
string to_string() const;
```

## HTTPClient

```cpp
HTTPClient(Net::EndPoint server, SSL::Context *ctx=nullptr);
int post(const URL& url, Message& request, Message& response);
```

## MessageInterface

```cpp
MessageInterface(CONTEXT& context, const XML::Element& config,  // <soap>
                 Message::Broker<CONTEXT>& broker, const string& server_name,
                 SSL::Context *ssl_ctx=0);
bool is_listening() const;   // all servers created are listening (true if none);
                             // ones that can't are logged as errors ("Can't listen...")
void set_cors_origin(const string& pattern="*");
void shutdown();
```

Config: `<server port= address=.../>` (HTTP), `<ssl-server port= address=.../>` (HTTPS, only created if `ssl_ctx` given); address defaults to `localhost`.
