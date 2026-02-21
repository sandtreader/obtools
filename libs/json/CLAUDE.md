# CLAUDE.md - ObTools::JSON Library

## Overview

`ObTools::JSON` is a JSON parser/writer with CBOR binary format support. Provides a `Value` type representing all JSON types, a streaming parser, and CBOR encode/decode. Lives under `namespace ObTools::JSON`.

**Header:** `ot-json.h`
**Dependencies:** `ot-lex`, `ot-text`, `ot-chan`

## Value::Type Enum

```cpp
enum Type {
  UNDEFINED, NULL_, NUMBER, INTEGER, STRING,
  OBJECT, ARRAY, TRUE_, FALSE_, BINARY, BREAK
};
```

## Value Class

**Static:** `Value::none` - singleton undefined value.

**Public members:** `type`, `f` (double), `n` (int64_t), `s` (string), `o` (map\<string,Value\>), `a` (vector\<Value\>).

**Constructors:**
```cpp
Value();                                    // UNDEFINED
Value(Type _type);                          // typed
Value(float/double _f);                     // NUMBER
Value(int32_t/uint32_t/int64_t/uint64_t);   // INTEGER
Value(const string& _s);  Value(const char *_s);  // STRING
Value(const vector<unsigned char>& data);   // BINARY
Value(const vector<byte>& data);            // BINARY
```

**Object methods:**
```cpp
Value& put(const string& name, const Value& v);   // set property, returns added value
Value& set(const string& name, const Value& v);   // set property, returns *this (chainable)
```

**Array methods:**
```cpp
Value& add(const Value& v);   // append, returns added value
size_t size() const;           // array length (0 if not array)
```

**Access:**
```cpp
const Value& get(const string& property) const;   // object property (Value::none if missing)
Value& get(const string& property);
const Value& get(unsigned int index) const;        // array element
Value& get(unsigned int index);
const Value& operator[](const string& property) const;
const Value& operator[](unsigned int index) const;
// + mutable versions
```

**Type conversions:**
```cpp
string as_str(const string& def="") const;
int64_t as_int(int64_t def=0) const;
double as_float(double def=0.0) const;
vector<byte> as_binary() const;
bool is_true() const;
```

**Output:**
```cpp
void write_to(ostream& s, bool pretty=false, int indent=0) const;
string str(bool pretty=false) const;
string cbor() const;                  // encode to CBOR
```

**Operators:** `operator!()` (undefined check), `operator==`, `operator!=`, `operator<<`.

## Parser

```cpp
Parser(istream& input);
Value read_value();          // parse one JSON value from stream
```

## CBORWriter / CBORReader

```cpp
CBORWriter(Channel::Writer& writer);
void encode(const Value& v);
void open_indefinite_array();
void close_indefinite_array();

CBORReader(Channel::Reader& reader);
JSON::Value decode();
bool open_indefinite_array();   // check for indefinite array (0x9f)
```

## Exception

```cpp
struct Exception { string error; };
```

## File Layout

```
ot-json.h              - Public header
value.cc               - Value class
parser.cc              - JSON parser
cbor-reader.cc         - CBOR decoder
cbor-writer.cc         - CBOR encoder
test-value.cc          - Value tests
test-parser.cc         - Parser tests
test-cbor-reader.cc    - CBOR reader tests
test-cbor-writer.cc    - CBOR writer tests
test-real-data.cc      - Real-world JSON tests
```
