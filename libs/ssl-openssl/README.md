# ObTools::SSL_OpenSSL

OpenSSL implementation of the abstract `SSL::Context` and `SSL::Connection` interfaces. Provides TLS/SSL support for ObTools networking classes.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Features

- **OpenSSL integration**: wraps `SSL_CTX` and `SSL` for TLS/SSL connections
- **Certificate management**: load certificates and keys from PEM files or objects
- **Peer verification**: configurable CA paths, CN verification, client certificates
- **SNI support**: Server Name Indication for virtual hosting
- **XML configuration**: create SSL contexts from XML config
- **Factory methods**: `create()` and `create_anonymous()` for common setups

## Dependencies

- `ot-ssl` - Abstract SSL interface
- `ot-crypto` - Cryptographic primitives (Certificate, RSAKey)
- `ext-pkg-openssl` - OpenSSL library

## Quick Start

```cpp
#include "ot-ssl-openssl.h"
using namespace ObTools;
```

### Simple SSL Client

```cpp
SSL_OpenSSL::Context ctx;
ctx.set_default_verify_paths();  // use system CA certificates

SSL::TCPClient client(&ctx, Net::EndPoint("www.example.com", 443));
if (!client)
{
  SSL_OpenSSL::Context::log_errors("Connection failed");
  return;
}

client.write("GET / HTTP/1.0\r\nHost: www.example.com\r\n\r\n");
string response;
client.readall(response);
```

### SSL Client with SNI

```cpp
SSL_OpenSSL::Context ctx;
ctx.set_default_verify_paths();
ctx.set_sni_hostname("www.example.com");

SSL::TCPClient client(&ctx, Net::EndPoint("www.example.com", 443));
```

### SSL Client with Peer Verification

```cpp
SSL_OpenSSL::Context ctx;
ctx.set_verify_paths("/etc/ssl/certs/ca-certificates.crt");
ctx.enable_peer_verification(true);  // force verification

// Verify specific CN
ctx.verify_common_name = "www.example.com";
ctx.enable_peer_verification(true, true);  // force + CN check

SSL::TCPClient client(&ctx, Net::EndPoint("www.example.com", 443));
```

### SSL Server with Certificates

```cpp
// Load certificate and key
SSL_OpenSSL::Context ctx;
ctx.use_certificate("/path/to/server.crt");
ctx.use_private_key("/path/to/server.key", "passphrase");

// Or from Crypto objects
Crypto::Certificate cert("/path/to/server.crt");
Crypto::RSAKey key("/path/to/server.key");
ctx.use_certificate(cert);
ctx.use_private_key(key);

// Optional: require client certificates
ctx.set_client_ca_file("/path/to/ca.crt");
ctx.enable_peer_verification(true);

// Create SSL server
class MyServer: public SSL::TCPServer
{
public:
  MyServer(SSL::Context *ctx, int port): TCPServer(ctx, port) {}

  void process(SSL::TCPSocket& s, const SSL::ClientDetails& client) override
  {
    if (!client.cert_cn.empty())
      cout << "Client cert CN: " << client.cert_cn << endl;

    // handle connection...
  }
};

MyServer server(&ctx, 8443);
server.run();
```

### Certificate Chain

```cpp
SSL_OpenSSL::Context ctx;

// Primary certificate
ctx.use_certificate("/path/to/server.crt");

// Intermediate certificates (chain)
ctx.use_certificate("/path/to/intermediate.crt", true);  // is_extra=true

ctx.use_private_key("/path/to/server.key");
```

### Create from XML Configuration

```xml
<ssl>
  <certificate>/path/to/cert.pem</certificate>
  <key>/path/to/key.pem</key>
  <verify-peer force="yes" common-name="expected.cn"/>
  <ca-file>/path/to/ca-bundle.crt</ca-file>
</ssl>
```

```cpp
XML::Element& ssl_config = /* from XML config */;

// Full context with certificate and key
SSL_OpenSSL::Context *ctx = SSL_OpenSSL::Context::create(ssl_config, "key-passphrase");

// Anonymous context (no certificate - for clients that don't need one)
SSL_OpenSSL::Context *ctx = SSL_OpenSSL::Context::create_anonymous(ssl_config);

// Configure verification on existing context
SSL_OpenSSL::Context::configure_verification(ctx, ssl_config);
```

### Error Handling

```cpp
// Log OpenSSL error queue
SSL_OpenSSL::Context::log_errors("Failed to load certificate");
```

## API Reference

### Context

| Method | Returns | Description |
|--------|---------|-------------|
| `Context()` | | Create new SSL_CTX with TLS_method() |
| `use_certificate(cert, extra)` | `void`/`bool` | Load certificate (PEM or object) |
| `use_private_key(key, pass)` | `void`/`bool` | Load private key (PEM or object) |
| `enable_peer_verification(force, cn)` | `void` | Enable peer certificate checks |
| `set_verify_paths(file, dir)` | `void` | Set CA certificate locations |
| `set_default_verify_paths()` | `void` | Use system default CA paths |
| `set_client_ca_file(file)` | `void` | Request client certificates |
| `set_sni_hostname(host)` | `void` | Set SNI hostname |
| `set_session_id_context(s)` | `void` | Set session ID context |
| `accept_connection(fd)` | `Connection*` | SSL_accept on fd |
| `connect_connection(fd)` | `Connection*` | SSL_connect on fd |
| `create(xml, pass)` | `Context*` | Factory from XML config |
| `create_anonymous(xml)` | `Context*` | Factory without key/cert |
| `configure_verification(ctx, xml)` | `void` | Configure verification from XML |
| `log_errors(text)` | `void` | Log OpenSSL error queue |

### Connection

| Method | Returns | Description |
|--------|---------|-------------|
| `cread(buf, count)` | `ssize_t` | SSL_read wrapper |
| `cwrite(buf, count)` | `ssize_t` | SSL_write wrapper |
| `get_peer_cn()` | `string` | Peer X509 common name |
| `set_sni_hostname(host)` | `void` | Set SNI hostname |

## Build

```
NAME      = ot-ssl-openssl
TYPE      = lib
DEPENDS   = ot-ssl ot-crypto ext-pkg-openssl
PLATFORMS = posix web
```

## License

Copyright (c) 2008 Paul Clark. MIT License.
