# CLAUDE.md - ObTools::DNS Library

## Overview

`ObTools::DNS` provides portable DNS lookups for TXT, CERT, and other record types beyond standard `gethostbyname()`. Lives under `namespace ObTools::DNS`.

**Header:** `ot-dns.h`
**Dependencies:** `ot-log`, `ot-chan`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Resolver` | DNS query resolver for TXT, CERT, and raw RR types |

## Query Types

```cpp
enum Type {
  TYPE_TXT  = 16,   // Text record
  TYPE_CERT = 37    // Certificate record
};
```

## Resolver

```cpp
Resolver();
string query(const string& domain, Type type, const string& type_name = "");
string query_txt(const string& domain);    // returns repacked TXT data
string query_cert(const string& domain);   // returns DER certificate data
```

## Platform Notes

- **Unix/Linux:** Uses `libresolv` with RFC1035 DNS message parsing
- **Windows:** Uses `DnsQuery` API
- Returns empty string on failure
- `query()` returns raw RDATA from first answer section
- `query_txt()` repacks TXT data (removes length prefixes)
- `query_cert()` returns binary DER format certificate
