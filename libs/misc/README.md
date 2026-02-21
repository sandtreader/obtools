# ObTools::Misc

A collection of miscellaneous utility classes for C++17: MD5 hashing, CRC/CRC32, property lists, random number generation, UUID, hex dumping, range sets, and hash-based interpolation.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Features

- **Hashing**: MD5 message digest, CRC16 (multiple algorithms), CRC32/CRC32C
- **Property lists**: Named string map with typed access, string interpolation, environment import
- **Random numbers**: Pseudo-random binary, hex, integer generation
- **UUID**: 128-bit unique identifiers with string/hex/base64 formats
- **Hex dump**: Configurable hex/ASCII data dumper
- **Range sets**: Non-overlapping range management with full set algebra (union, intersection, difference, inverse)
- **Hash interpolation**: XML-configured hash-based property augmentation

## Dependencies

- `ot-xml` - XML parsing (for HashInterpolator and UInt64RangeSet XML serialisation)
- `ot-chan` - Binary channel protocol (for UInt64RangeSet binary serialisation)
- `ot-text` - Text utilities (for PropertyList string parsing)

## Quick Start

```cpp
#include "ot-misc.h"
using namespace ObTools;
```

### MD5 Hashing

```cpp
Misc::MD5 md5;

// One-shot to hex string
string hex = md5.sum("Hello, World!");
// "65a8e27d8879283831b664bd8b7f0ad4"

// One-shot to base64
string b64 = md5.sum_base64("Hello, World!");

// One-shot to 64-bit hash
uint64_t h = md5.hash_to_int("Hello, World!");

// Incremental hashing
md5.initialise();
md5.update(data1, len1);
md5.update(data2, len2);
unsigned char digest[16];
md5.finalise(digest);
```

### CRC16

```cpp
// Standard CRC16
Misc::CRC crc;
uint16_t result = crc.calculate("Hello");

// CCITT variant
Misc::CRC ccitt(Misc::CRC::ALGORITHM_CCITT);
uint16_t result = ccitt.calculate(data, length);

// Reflected (network byte order)
Misc::CRC crc_ref(Misc::CRC::ALGORITHM_CRC16, true);

// With final XOR
Misc::CRC crc_flip(Misc::CRC::ALGORITHM_CRC16, false, true);
```

Available algorithms: `ALGORITHM_CRC16`, `ALGORITHM_CCITT`, `ALGORITHM_CCITT_ZERO`, `ALGORITHM_CCITT_MOD`.

### CRC32

```cpp
// Standard CRC32 (reflected + flipped by default, matching zlib/gzip)
Misc::CRC32 crc;
uint32_t result = crc.calculate("Hello");

// CRC32C (Castagnoli)
Misc::CRC32 crc32c(Misc::CRC32::ALGORITHM_CRC32C);

// Incremental CRC32
uint32_t running = crc.initialiser();
running = crc.consume(chunk1, len1, running);
running = crc.consume(chunk2, len2, running);
uint32_t final = crc.finalise(running);
```

### Property Lists

A string-to-string map with typed accessors and interpolation:

```cpp
// Create from scratch
Misc::PropertyList props;
props.add("host", "localhost");
props.add("port", 8080);
props.add("debug", (uint64_t)123456789);
props.add_bool("verbose", true);

// Query with typed access
string host = props.get("host", "127.0.0.1");
string host = props["host"];                    // same, no default
int port    = props.get_int("port", 80);
bool debug  = props.get_bool("verbose", false);
double rate = props.get_real("rate", 1.0);
bool exists = props.has("host");

// Parse from delimited string
Misc::PropertyList props("host=localhost,port=8080");

// Serialise back
string s = props.str();                // "host=localhost,port=8080"
string s = props.str(';', '\'');       // custom separator and quote

// Dump (human-readable)
props.dump(cout);
//     host = localhost
//     port = 8080
cout << props;  // same thing
```

#### String Interpolation

```cpp
Misc::PropertyList vars;
vars.add("name", "World");
vars.add("count", 42);

string result = vars.interpolate("Hello, ${name}! Count: ${count}");
// "Hello, World! Count: 42"
```

#### Environment Variables

```cpp
Misc::PropertyList env;
env.fill_from_environment();
string home = env.get("HOME");
```

### Random Numbers

```cpp
Misc::Random rng;

// Raw binary
unsigned char buf[32];
rng.generate_binary(buf, 32);

vector<byte> bytes = rng.generate_binary(32);

// Hex string
string hex = rng.generate_hex(16);  // 16 random bytes as 32-char hex

// Integers
uint32_t r32 = rng.generate_32();
uint64_t r64 = rng.generate_64();
unsigned int dice = rng.generate_up_to(6);  // [0, 6)
```

### UUID

```cpp
// Create zero UUID
Misc::UUID uuid;

// Parse from string
Misc::UUID uuid("550e8400-e29b-41d4-a716-446655440000");

// Generate random
Misc::UUID uuid;
uuid.randomise();

// Output formats
string standard = uuid.get_str();       // "550e8400-e29b-41d4-a716-446655440000"
string hex      = uuid.get_hex_str();   // "550e8400e29b41d4a716446655440000"
string b64      = uuid.get_base64_str();

// Validity check
if (uuid) { /* non-zero */ }

// Stream output
cout << uuid;  // prints standard format
```

### Hex Dumper

```cpp
// Default: 16 bytes/line, split every 4, with ASCII
Misc::Dumper dumper(cout);
dumper.dump(data, length);
// Output:
// 48656c6c 6f2c2057 6f726c64 21000000  Hello, World!...

// Custom format
Misc::Dumper compact(cout, 8, 0, false);  // 8 bytes/line, no split, no ASCII
compact.dump(data, length);

// Dump string or vector
dumper.dump(my_string);
dumper.dump(my_byte_vector);
```

### Range Sets

Manage sets of non-overlapping ranges with full set algebra:

```cpp
// Create a range set with a known total size
Misc::RangeSet<int, int> rs(100);

// Insert ranges
rs.insert(0, 10);    // [0, 10)
rs.insert(20, 30);   // [20, 50)
rs.insert(5, 10);    // [5, 15) - merges with [0, 10) -> [0, 15)

// Query
bool has = rs.contains(5, 5);      // true
int covered = rs.coverage();        // total length covered
int n = rs.count();                 // number of distinct ranges
bool done = rs.is_complete();       // covers entire [0, 100)?
int pct = rs.percentage_complete();
string bar = rs.gauge(40);          // ASCII progress bar

// Remove
rs.remove(10, 5);  // punch hole at [10, 15)

// Set operations
auto u = rs1.set_union(rs2);        // also rs1 + rs2
auto d = rs1.difference(rs2);       // also rs1 - rs2
auto i = rs1.intersection(rs2);     // also rs1 ^ rs2
auto inv = rs.inverse();            // complement within [0, end)

// Multi-set intersection
auto common = Misc::RangeSet<int,int>::intersection({rs1, rs2, rs3});

// Iterate ranges
for (const auto& r : rs)
  cout << r.start << "-" << r.end() << endl;
```

### UInt64RangeSet

64-bit specialisation with serialisation support:

```cpp
// Create and populate
Misc::UInt64RangeSet rs(1000);  // total size 1000
rs.insert(0, 100);
rs.insert(500, 200);

// String serialisation
string s = rs.str();             // "0+100,500+200"
Misc::UInt64RangeSet rs2(s, 1000);  // parse from string
rs2.read(another_string);

// XML serialisation
rs.add_to_xml(parent_element, "range");
// produces: <range start="0" length="100"/><range start="500" length="200"/>
rs2.read_from_xml(parent_element, "range");

// Binary serialisation (via Channel)
rs.write(channel_writer);
rs2.read(channel_reader);

// Debug output
rs.dump(cout);
cout << rs;
```

### Hash Interpolator

Augment property lists with hash-derived values from XML configuration:

```cpp
// XML config:
// <hashes>
//   <hash name="bucket" modulus="16" pattern="${key}"/>
// </hashes>

XML::Parser parser;
parser.read_from(xml);
Misc::HashInterpolator hi(parser.get_root());

// Or build programmatically
Misc::HashInterpolator hi;
hi.add_hash("bucket", 16, "${key}");

// Augment properties
Misc::PropertyList props;
props.add("key", "some-data");
hi.augment(props);
// props now contains "bucket" with a hash-derived value
```

## API Reference

### MD5

| Method | Returns | Description |
|--------|---------|-------------|
| `MD5()` | | Constructor (auto-initialises) |
| `initialise()` | `void` | Reset for reuse |
| `update(buf, len)` | `void` | Feed data incrementally |
| `finalise(digest)` | `void` | Produce 16-byte digest |
| `sum(text, digest)` | `void` | One-shot to raw digest |
| `sum(text)` | `string` | One-shot to hex string |
| `sum_base64(text)` | `string` | One-shot to base64 |
| `hash_to_int(text)` | `uint64_t` | One-shot to 64-bit hash |

### CRC / CRC32

| Method | Returns | Description |
|--------|---------|-------------|
| `calculate(data, length)` | `crc_t` | Compute CRC |
| `calculate(string)` | `crc_t` | Compute CRC from string |
| `initialiser()` | `crc_t` | (CRC32 only) Initial value for incremental |
| `consume(data, len, crc)` | `crc_t` | (CRC32 only) Incremental computation |
| `finalise(crc)` | `crc_t` | (CRC32 only) Finalise incremental |

### PropertyList

| Method | Returns | Description |
|--------|---------|-------------|
| `add(name, value)` | `void` | Add string/int/uint64 property |
| `add_bool(name, value)` | `void` | Add boolean property |
| `has(name)` | `bool` | Check existence |
| `get(name, def)` | `string` | Get string value |
| `operator[](name)` | `string` | Get string value (no default) |
| `get_int(name, def)` | `int` | Get integer value |
| `get_bool(name, def)` | `bool` | Get boolean value |
| `get_real(name, def)` | `double` | Get double value |
| `interpolate(text)` | `string` | Substitute `${name}` variables |
| `str(sep, quote)` | `string` | Serialise to delimited string |
| `dump(stream, prefix, sep)` | `void` | Human-readable output |
| `fill_from_environment()` | `void` | Import environment variables |

### Random

| Method | Returns | Description |
|--------|---------|-------------|
| `generate_binary(p, n)` | `void` | Fill buffer with n random bytes |
| `generate_binary(n)` | `vector<byte>` | Return n random bytes |
| `generate_hex(n)` | `string` | Hex string of n random bytes |
| `generate_32()` | `uint32_t` | Random 32-bit integer |
| `generate_64()` | `uint64_t` | Random 64-bit integer |
| `generate_up_to(n)` | `unsigned int` | Random in [0, n) |

### UUID

| Method | Returns | Description |
|--------|---------|-------------|
| `UUID()` | | Zero UUID |
| `UUID(string)` | | Parse from standard format |
| `get_str()` | `string` | Standard format with hyphens |
| `get_hex_str()` | `string` | 32-char hex, no hyphens |
| `get_base64_str()` | `string` | Base64 encoded |
| `randomise()` | `void` | Generate random UUID |
| `operator bool()` | `bool` | Non-zero check |

### Dumper

| Method | Description |
|--------|-------------|
| `Dumper(stream, width, split, ascii)` | Constructor |
| `dump(block, length)` | Dump raw data |
| `dump(string)` | Dump string |
| `dump(vector<byte>)` | Dump byte vector |

### RangeSet<T, L>

| Method | Returns | Description |
|--------|---------|-------------|
| `insert(start, length)` | `void` | Add a range |
| `remove(start, length)` | `void` | Remove a range |
| `clear()` | `void` | Remove all ranges |
| `contains(start, length)` | `bool` | Check if range is covered |
| `coverage()` | `length_t` | Total covered length |
| `count()` | `size_type` | Number of distinct ranges |
| `is_complete()` | `bool` | Full coverage? |
| `percentage_complete()` | `int` | Coverage percentage |
| `gauge(length)` | `string` | ASCII progress bar |
| `set_union(o)` / `operator+` | `RangeSet` | Union |
| `difference(o)` / `operator-` | `RangeSet` | Difference |
| `intersection(o)` / `operator^` | `RangeSet` | Intersection |
| `inverse()` | `RangeSet` | Complement |

## Build

```
NAME    = ot-misc
TYPE    = lib
DEPENDS = ot-xml ot-chan ot-text
```

## License

Copyright (c) 2003 Paul Clark. MIT License.
