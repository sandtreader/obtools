# ObTools::JSON

A JSON parser/writer for C++17 with CBOR binary format support. Provides a flexible `Value` type representing all JSON types, a streaming parser, and CBOR encode/decode capabilities.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Features

- **Value type**: represents all JSON types (null, bool, number, integer, string, object, array) plus binary (CBOR)
- **Streaming parser**: parse JSON from any `istream`
- **Pretty printing**: human-readable indented output
- **CBOR support**: encode/decode to Concise Binary Object Representation
- **Fluent API**: chainable `set()` for building objects

## Dependencies

- `ot-lex` - Lexical analysis
- `ot-text` - Text utilities
- `ot-chan` - Binary channel protocol (for CBOR)

## Quick Start

```cpp
#include "ot-json.h"
using namespace ObTools;
```

### Creating Values

```cpp
// Primitives
JSON::Value null_val(JSON::Value::NULL_);
JSON::Value num(3.14);
JSON::Value integer(42);
JSON::Value str("hello");
JSON::Value t(JSON::Value::TRUE_);
JSON::Value f(JSON::Value::FALSE_);

// Object
JSON::Value obj(JSON::Value::OBJECT);
obj.put("name", "Alice");
obj.put("age", 30);
obj.put("active", JSON::Value(JSON::Value::TRUE_));

// Chainable object building
JSON::Value config(JSON::Value::OBJECT);
config.set("host", "localhost")
      .set("port", 8080)
      .set("debug", JSON::Value(JSON::Value::FALSE_));

// Array
JSON::Value arr(JSON::Value::ARRAY);
arr.add(1);
arr.add(2);
arr.add("three");

// Nested
JSON::Value user(JSON::Value::OBJECT);
user.put("name", "Bob");
JSON::Value& tags = user.put("tags", JSON::Value(JSON::Value::ARRAY));
tags.add("admin");
tags.add("user");
```

### Parsing JSON

```cpp
// From string
istringstream iss(R"({"name": "Alice", "age": 30})");
JSON::Parser parser(iss);
JSON::Value val = parser.read_value();

// From file
ifstream file("data.json");
JSON::Parser fparser(file);
JSON::Value data = fparser.read_value();

// Error handling
try
{
  JSON::Value v = parser.read_value();
}
catch (JSON::Exception& e)
{
  cerr << "Parse error: " << e.error << endl;
}
```

### Accessing Values

```cpp
JSON::Value obj = /* parsed object */;

// Object property access
const JSON::Value& name = obj["name"];       // returns Value::none if missing
string s = obj["name"].as_str();             // "Alice"
int64_t n = obj["age"].as_int();             // 30
double f = obj["score"].as_float(0.0);       // with default
bool b = obj["active"].is_true();

// Check for missing
if (!obj["missing"]) { /* it's undefined */ }

// Array access
const JSON::Value& first = arr[0];
size_t len = arr.size();

// Iterate array
for (unsigned int i = 0; i < arr.size(); i++)
{
  const JSON::Value& item = arr[i];
  // ...
}

// Iterate object
for (const auto& [key, val] : obj.o)
{
  cout << key << ": " << val.as_str() << endl;
}
```

### Output

```cpp
JSON::Value obj(JSON::Value::OBJECT);
obj.put("name", "Alice");
obj.put("age", 30);

// Compact (default)
string compact = obj.str();
// {"name":"Alice","age":30}

// Pretty-printed
string pretty = obj.str(true);
// {
//   "name": "Alice",
//   "age": 30
// }

// To stream
obj.write_to(cout, true);
cout << obj;  // compact via operator<<
```

### CBOR Binary Format

```cpp
// Encode to CBOR
JSON::Value data(JSON::Value::OBJECT);
data.put("key", "value");
string cbor_bytes = data.cbor();

// Decode from CBOR
Channel::BlockReader reader(cbor_data, cbor_length);
JSON::CBORReader cbor_reader(reader);
JSON::Value decoded = cbor_reader.decode();

// Encode with streaming writer
Channel::BlockWriter writer;
JSON::CBORWriter cbor_writer(writer);
cbor_writer.encode(data);

// Indefinite-length arrays
cbor_writer.open_indefinite_array();
cbor_writer.encode(JSON::Value(1));
cbor_writer.encode(JSON::Value(2));
cbor_writer.close_indefinite_array();
```

### Type Conversions

```cpp
JSON::Value v;

// String conversion (works across types)
v = JSON::Value(42);
string s = v.as_str();       // "42"

// Integer conversion (parses numeric strings)
v = JSON::Value("123");
int64_t n = v.as_int();      // 123

// Float conversion (promotes integers, parses strings)
v = JSON::Value(42);
double f = v.as_float();     // 42.0

// Binary data
vector<byte> data = {byte{0x01}, byte{0x02}};
v = JSON::Value(data);
vector<byte> back = v.as_binary();
```

### Value Comparison

```cpp
JSON::Value a(42);
JSON::Value b(42);
JSON::Value c("hello");

a == b;  // true - same type and value
a != c;  // true - different types

// Validity check (not the same as truthiness!)
JSON::Value undef;
if (!undef) { /* undefined */ }

// FALSE and NULL are still "valid"
JSON::Value f(JSON::Value::FALSE_);
if (!f) { /* NOT entered - FALSE is a valid value */ }
```

## API Reference

### Value

| Method | Returns | Description |
|--------|---------|-------------|
| `put(name, v)` | `Value&` | Set object property, returns added value |
| `set(name, v)` | `Value&` | Set object property, returns `*this` (chainable) |
| `add(v)` | `Value&` | Append to array, returns added value |
| `get(name)` | `Value&` | Get object property (`Value::none` if missing) |
| `get(index)` | `Value&` | Get array element |
| `operator[](name)` | `Value&` | Bracket access for objects |
| `operator[](index)` | `Value&` | Bracket access for arrays |
| `size()` | `size_t` | Array length (0 if not array) |
| `as_str(def)` | `string` | Convert to string |
| `as_int(def)` | `int64_t` | Convert to integer |
| `as_float(def)` | `double` | Convert to double |
| `as_binary()` | `vector<byte>` | Convert to binary |
| `is_true()` | `bool` | Truthy check |
| `str(pretty)` | `string` | Serialise to JSON string |
| `cbor()` | `string` | Encode to CBOR |
| `write_to(s, pretty, indent)` | `void` | Write to stream |
| `operator!()` | `bool` | Check if UNDEFINED |
| `operator==` / `operator!=` | `bool` | Value equality |

### Parser

| Method | Returns | Description |
|--------|---------|-------------|
| `Parser(istream&)` | | Construct from input stream |
| `read_value()` | `Value` | Parse one JSON value |

### CBOR

| Class | Method | Description |
|-------|--------|-------------|
| `CBORWriter` | `encode(v)` | Encode value to CBOR |
| `CBORWriter` | `open_indefinite_array()` | Start indefinite array |
| `CBORWriter` | `close_indefinite_array()` | End indefinite array |
| `CBORReader` | `decode()` | Decode one CBOR value |
| `CBORReader` | `open_indefinite_array()` | Check for indefinite array |

## Build

```
NAME    = ot-json
TYPE    = lib
DEPENDS = ot-lex ot-text ot-chan
```

## License

Copyright (c) 2017 Paul Clark. MIT License.
