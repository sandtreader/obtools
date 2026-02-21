# CLAUDE.md - ObTools::SSL_OpenSSL Library

## Overview

`ObTools::SSL_OpenSSL` provides the concrete OpenSSL implementation of the abstract `SSL::Context` and `SSL::Connection` interfaces. Lives under `namespace ObTools::SSL_OpenSSL`.

**Header:** `ot-ssl-openssl.h`
**Dependencies:** `ot-ssl`, `ot-crypto`, `ext-pkg-openssl`
**Platforms:** posix, web

## Context

Wraps `SSL_CTX`. Factory for SSL connections.

```cpp
Context();

// Certificate/key loading
void use_certificate(const Crypto::Certificate& cert, bool is_extra=false);
bool use_certificate(const string& pem, bool is_extra=false);
void use_private_key(Crypto::RSAKey& rsa);
bool use_private_key(const string& pem, const string& pass_phrase="");

// Peer verification
void enable_peer_verification(bool force=false, bool common_name=false);
void set_verify_paths(const string& ca_file="", const string& ca_dir="");
void set_default_verify_paths();
void set_client_ca_file(const string& ca_file);
string verify_common_name;   // CN to verify against

// SNI
void set_sni_hostname(const string& host);

// Session
void set_session_id_context(const string& s);

// Connection creation (implements SSL::Context)
Connection *accept_connection(int fd);
Connection *connect_connection(int fd);

// Factory from XML config
static Context *create(const XML::Element& ssl_e, string pass_phrase="");
static Context *create_anonymous(const XML::Element& ssl_e);
static void configure_verification(Context *ctx, const XML::Element& ssl_e);

// Error logging
static void log_errors(const string& text);
```

## Connection

Wraps `::SSL`. Implements `SSL::Connection`.

```cpp
Connection(::SSL *ssl);
ssize_t cread(void *buf, size_t count);    // SSL_read
ssize_t cwrite(const void *buf, size_t count);  // SSL_write
string get_peer_cn();                       // peer X509 CN
void set_sni_hostname(const string& host);
```

## XML Configuration

The `Context::create()` factory reads this XML format:

```xml
<ssl>
  <certificate>/path/to/cert.pem</certificate>
  <key>/path/to/key.pem</key>
  <verify-peer force="yes" common-name="expected.cn"/>
  <ca-file>/path/to/ca.pem</ca-file>
  <ca-dir>/path/to/certs/</ca-dir>
</ssl>
```

## File Layout

```
ot-ssl-openssl.h     - Public header
context.cc           - Context (SSL_CTX) implementation
connection.cc        - Connection (SSL) implementation
legacy-test-client.cc - Legacy SSL client test
legacy-test-server.cc - Legacy SSL server test
```
