# ObTools::DNS

Portable DNS lookups for TXT, CERT, and other record types that cannot be obtained from standard `gethostbyname()`.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-dns.h"
using namespace ObTools;

DNS::Resolver resolver;

// Query TXT record
string txt = resolver.query_txt("example.com");
if (!txt.empty())
  cout << "TXT: " << txt << endl;

// Query CERT record (returns binary DER data)
string cert = resolver.query_cert("example.com");
if (!cert.empty())
  cout << "Got " << cert.size() << " bytes of certificate data" << endl;

// Raw query for any RR type
string raw = resolver.query("example.com", DNS::TYPE_TXT, "TXT");
```

## Build

```
NAME    = ot-dns
TYPE    = lib
DEPENDS = ot-log ot-chan
```

## License

Copyright (c) 2008 Paul Clark. MIT License.
