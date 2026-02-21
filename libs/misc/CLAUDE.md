# CLAUDE.md - ObTools::Misc Library

## Overview

`ObTools::Misc` is a collection of miscellaneous utility classes: MD5 hashing, CRC/CRC32, property lists, random number generation, UUID, hex dumping, range sets, and hash-based interpolation. Lives under `namespace ObTools::Misc`.

**Header:** `ot-misc.h`
**Dependencies:** `ot-xml`, `ot-chan`, `ot-text`

## Key Classes

| Class | Purpose |
|-------|---------|
| `MD5` | MD5 message digest |
| `CRC` | 16-bit CRC calculator (CRC16, CCITT variants) |
| `CRC32` | 32-bit CRC calculator (CRC32, CRC32C) |
| `PropertyList` | Named string map with typed access and interpolation |
| `Random` | Pseudo-random number generator |
| `UUID` | 128-bit universally unique identifier |
| `Dumper` | Hex/ASCII data dump to stream |
| `RangeSet<T,L>` | Generic set of ranges with set operations |
| `UInt64RangeSet` | 64-bit specialisation with XML/binary/string serialisation |
| `HashInterpolator` | Hash-based property augmentation from XML config |

## MD5

```cpp
MD5();
void initialise();                              // reset for reuse
void update(const char *buf, unsigned len);     // feed data
void finalise(unsigned char digest[16]);         // produce digest
void sum(const string& text, unsigned char digest[16]);  // one-shot to raw
string sum(const string& text);                 // one-shot to hex string
string sum_base64(const string& text);          // one-shot to base64
uint64_t hash_to_int(const string& text);       // one-shot to 64-bit hash
```

## CRC / CRC32

```cpp
// CRC16
CRC(Algorithm alg = ALGORITHM_CRC16, bool reflected = false, bool flip = false);
crc_t calculate(const unsigned char *data, size_t length);
crc_t calculate(const string& data);
// Algorithms: ALGORITHM_CRC16, ALGORITHM_CCITT, ALGORITHM_CCITT_ZERO, ALGORITHM_CCITT_MOD

// CRC32
CRC32(Algorithm alg = ALGORITHM_CRC32, bool reflected = true, bool flip = true);
crc_t calculate(const unsigned char *data, size_t length);
crc_t calculate(const string& data);
crc_t initialiser() const;                            // get initial CRC value
crc_t consume(const unsigned char *data, size_t length, crc_t crc) const;  // incremental
crc_t finalise(crc_t crc) const;                      // finalise incremental CRC
// Algorithms: ALGORITHM_CRC32, ALGORITHM_CRC32C
```

## PropertyList

Inherits `map<string, string>`.

```cpp
PropertyList();
PropertyList(const string& str, char sep=',', char quote='"');   // parse from string
PropertyList(const map<string, string>& o);

// Add values
void add(const string& name, const string& value);
void add(const string& name, int value);
void add(const string& name, uint64_t value);
void add_bool(const string& name, bool value);

// Query values
bool has(const string& name) const;
string get(const string& name, const string& def="") const;
string operator[](const string& name) const;
int get_int(const string& name, int def=0) const;
bool get_bool(const string& name, bool def=false) const;
double get_real(const string& name, double def=0.0) const;

// String interpolation: replaces ${name} in text
string interpolate(const string& text) const;

// I/O
void dump(ostream& s, const string& prefix="    ", const string& separator=" = ") const;
string str(char sep=',', char quote='"');           // serialise to string
void fill_from_environment();                       // import all env vars
friend ostream& operator<<(ostream& s, const PropertyList& pl);
```

## Random

```cpp
Random();
void generate_binary(unsigned char *p, int n);    // fill buffer
vector<byte> generate_binary(int n);               // return vector
string generate_hex(int n);                        // hex string of n random bytes
uint32_t generate_32();                            // random 32-bit
uint64_t generate_64();                            // random 64-bit
unsigned int generate_up_to(unsigned int n);       // random in [0, n)
```

## UUID

Inherits `array<unsigned char, 16>`.

```cpp
UUID();                                            // zero UUID
UUID(b0, b1, ..., bf);                            // 16 explicit bytes
UUID(string str);                                  // parse "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
string get_str() const;                            // "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
string get_hex_str() const;                        // "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
string get_base64_str() const;                     // base64 encoded
void randomise();                                  // generate random UUID
explicit operator bool() const;                    // non-zero check
friend ostream& operator<<(ostream&, const UUID&); // outputs get_str()
```

## Dumper

```cpp
Dumper(ostream& sout, int width=16, int split=4, bool ascii=true);
void dump(const void *block, int length);
void dump(const string& s);
void dump(const vector<byte>& b);
```

## RangeSet<T, L>

Generic set of non-overlapping ranges with full set algebra.

```cpp
RangeSet(offset_t end_offset = offset_t());

// Insert/remove
void insert(offset_t start, length_t length=1);
void insert(const RangeSet& o);
void remove(offset_t start, length_t length=1);
void remove(const RangeSet& o);
void clear();

// Set operations
RangeSet set_union(const RangeSet& o) const;          // also operator+
RangeSet difference(const RangeSet& o) const;          // also operator-
RangeSet inverse() const;
RangeSet intersection(const RangeSet& o) const;        // also operator^
static RangeSet intersection(const list<RangeSet>& sets);

// Queries
bool contains(offset_t start, length_t length=1) const;
length_t coverage() const;                  // total covered length
size_type count() const;                    // number of ranges
bool is_complete() const;                   // covers [0, end_offset)
int percentage_complete();
string gauge(unsigned int length=50) const; // ASCII progress bar

// Iteration (delegates to inner set<Range>)
iterator begin(); iterator end();
const_iterator begin() const; const_iterator end() const;
```

## UInt64RangeSet

Inherits `RangeSet<uint64_t, uint64_t>`. Adds serialisation:

```cpp
UInt64RangeSet(length_t end_offset=0);
UInt64RangeSet(const string& text, length_t end_offset=0);  // parse from string

void read(const string& text);        string str() const;
void read_from_xml(const XML::Element& parent, const string& element_name="range");
void add_to_xml(XML::Element& parent, const string& element_name="range") const;
void read(Channel::Reader& chan);      void write(Channel::Writer& chan) const;
void dump(ostream& sout) const;
friend ostream& operator<<(ostream&, const UInt64RangeSet&);
```

## HashInterpolator

Augments a PropertyList with hash-derived values from XML config.

```cpp
HashInterpolator();
HashInterpolator(const XML::Element& root);        // load from XML
void add_hash(const string& name, unsigned int modulus, const string& pattern);
void augment(PropertyList& pl) const;              // add hash properties
list<Hash> get_hashes() const;
bool operator==(const HashInterpolator& o) const;
```

## File Layout

```
ot-misc.h               - Public header (all classes)
md5.cc                   - MD5 implementation
crc.cc                   - CRC16 implementation
crc32.cc                 - CRC32 implementation
proplist.cc              - PropertyList implementation
random.cc                - Random number generator
uuid.cc                  - UUID implementation
dumper.cc                - Hex dumper implementation
range.cc                 - RangeSet implementation
hash-interp.cc           - HashInterpolator implementation
test-hash-interp.cc      - HashInterpolator tests (gtest)
test-prop.cc             - PropertyList tests
test-range.cc            - RangeSet tests
test-uuid.cc             - UUID tests
Tupfile                  - Build configuration
```
