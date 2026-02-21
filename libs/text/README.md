# ObTools::Text

A string manipulation and encoding utility library for C++17. Provides whitespace handling, pattern matching, type conversions, multiple encoding schemes (Base64, Base58, Base36, Bech32), UTF-8 support, and CSV parsing.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Features

- **Whitespace handling**: stripping, condensing, indentation, canonicalisation
- **String splitting**: by delimiter, words, and lines
- **Pattern matching**: Unix glob-style with captures (`*`, `?`, `[abc]`, `[!abc]`)
- **Type conversions**: int, int64, float, bool, hex, fixed-point, binary-hex
- **Encoding/decoding**: Base64 (RFC + URL-safe), Base58, Base36, Base16Alpha, Bech32
- **UTF-8**: encode/decode, ISO-Latin1 conversion, diacritics stripping
- **CSV parsing**: with quoted fields and custom separators

## Dependencies

- `ot-gen` - ObTools generation utilities

## Quick Start

```cpp
#include "ot-text.h"
using namespace ObTools;
```

### Whitespace Handling

```cpp
// Canonicalise: trim + collapse internal whitespace to single spaces
string s = Text::canonicalise_space("  hello   world  ");  // "hello world"

// Remove all whitespace
string s = Text::remove_space("h e l l o");  // "hello"

// Strip leading/trailing blank lines (e.g. from XML)
string clean = Text::strip_blank_lines("\nsome text\n");

// Condense blank lines (remove edge blanks, collapse internal doubles)
string tidy = Text::condense_blank_lines("\n\nline1\n\n\nline2\n\n");
// "line1\n\nline2"

// Common indent detection and removal
string code = "  line1\n  line2\n    indented";
int indent = Text::get_common_indent(code);       // 2
string flat = Text::remove_indent(code, indent);
```

### Splitting

```cpp
// Split by delimiter (default: comma, with whitespace canonicalisation)
vector<string> fields = Text::split("a, b, c");
// {"a", "b", "c"}

// Custom delimiter, no canonicalisation
vector<string> parts = Text::split("a:b:c", ':', false);

// Limit number of fields (rest goes in last field)
vector<string> pair = Text::split("key=val1=val2", '=', true, 2);
// {"key", "val1=val2"}

// Split into words
vector<string> words = Text::split_words("  hello   world  ");
// {"hello", "world"}

// Split into lines
vector<string> lines = Text::split_lines("line1\nline2\n\nline3");
vector<string> nonblank = Text::split_lines("line1\n\nline2", true);

// Progressive word/line removal
string text = "hello world";
string first = Text::remove_word(text);  // first="hello", text="world"
```

### Pattern Matching

Unix glob-style patterns:

```cpp
// Simple match
bool ok = Text::pattern_match("*.txt", "readme.txt");      // true
bool ok = Text::pattern_match("data-?", "data-1");          // true
bool ok = Text::pattern_match("[abc]at", "bat");             // true
bool ok = Text::pattern_match("[!0-9]*", "hello");           // true

// Case-insensitive
bool ok = Text::pattern_match("*.TXT", "readme.txt", false);

// With captures (strings matched by *)
vector<string> matches;
Text::pattern_match("*-*.*", "foo-bar.txt", matches);
// matches = {"foo", "bar", "txt"}
```

### String Substitution

```cpp
string result = Text::subst("hello world", "world", "there");
// "hello there"

// Global replacement
string result = Text::subst("aaa", "a", "bb");
// "bbbbbb"
```

### Case Conversion

```cpp
string lower = Text::tolower("Hello World");  // "hello world"
string upper = Text::toupper("Hello World");  // "HELLO WORLD"
```

### Type Conversions

```cpp
// Integer <-> string
string s = Text::itos(42);          // "42"
int n = Text::stoi("42");           // 42

// 64-bit integer
string s = Text::i64tos(123456789012345ULL);
uint64_t n = Text::stoi64("123456789012345");

// Float <-> string
string s = Text::ftos(3.14159, 8, 2);        // "    3.14"
string s = Text::ftos(3.14159, 8, 2, true);  // "00003.14"
double f = Text::stof("3.14");

// Fixed-point integer
string s = Text::ifixtos(12345, 2);    // "123.45"
int n = Text::stoifix("123.45", 2);    // 12345

// Boolean (recognises T/t/Y/y/1 as true)
bool b = Text::stob("yes");  // true
bool b = Text::stob("no");   // false

// Hex
string h = Text::itox(255);          // "ff"
unsigned int n = Text::xtoi("ff");    // 255
string h = Text::i64tox(0xDEADBEEFULL);
uint64_t n = Text::xtoi64("deadbeef");

// Binary <-> hex
unsigned char data[] = {0xDE, 0xAD, 0xBE, 0xEF};
string hex = Text::btox(data, 4);         // "deadbeef"
string hex = Text::btox(binary_string);
string hex = Text::btox(byte_vector);

string bin = Text::xtob("deadbeef");       // binary string
unsigned char buf[4];
unsigned int len = Text::xtob("deadbeef", buf, 4);
```

### Base64 Encoding

```cpp
Text::Base64 b64;

// Encode
string encoded = b64.encode("Hello, World!");
string encoded = b64.encode(binary_data, length);
string encoded = b64.encode(byte_vector);
string encoded = b64.encode(uint64_value);

// Control line splitting (default 76 chars with \r\n)
string one_line = b64.encode("data", 0);           // no line breaks
string custom = b64.encode("data", 64, "\n  ");    // 64-char lines

// Decode
string binary;
b64.decode(encoded, binary);

vector<byte> bytes;
b64.decode(encoded, bytes);

uint64_t n;
b64.decode(encoded, n);

// Decode to raw buffer
unsigned char buf[1024];
size_t len = b64.decode(encoded, buf, sizeof(buf));
```

### Base64URL (URL-safe variant)

```cpp
Text::Base64URL b64url;
string encoded = b64url.encode("url-safe data");  // no padding, - and _ chars
```

### Base58 Encoding

```cpp
Text::Base58 b58;            // standard Bitcoin alphabet
Text::Base58 b58("custom_58_char_alphabet...");

vector<byte> data = {0x01, 0x02, 0x03};
string encoded = b58.encode(data);

vector<byte> decoded;
b58.decode(encoded, decoded);
```

### Base36 Encoding

```cpp
string encoded = Text::Base36::encode(123456789);
uint64_t n;
Text::Base36::decode(encoded, n);
```

### Base16Alpha (Vowel-Free)

Safe alphabet with no vowels (avoids accidental words): `bcdg hjkl mpqr svwz`.

```cpp
string encoded = Text::Base16Alpha::encode(42);
uint64_t n;
Text::Base16Alpha::decode(encoded, n);
```

### Bech32 Encoding

```cpp
vector<byte> data = {0x01, 0x02, 0x03};
string encoded = Text::Bech32::encode(data);

vector<byte> decoded;
Text::Bech32::decode(encoded, decoded);

// Decode as 5-bit data
vector<uint8_t> five_bit;
Text::Bech32::decode_as_5_bit(encoded, five_bit);
```

### UTF-8

```cpp
// Encode from Unicode
wchar_t chars[] = {0x00E9, 0};  // e-acute
string utf8 = Text::UTF8::encode(chars);

// Encode from ISO-Latin1
string utf8 = Text::UTF8::encode(latin1_string);

// Append single character
string s;
Text::UTF8::append(s, 0x00E9);

// Decode to wide chars
vector<wchar_t> unicode;
Text::UTF8::decode(utf8_string, unicode);

// Strip accents/diacritics (e-acute -> e, etc.)
string ascii = Text::UTF8::strip_diacritics("caf\xc3\xa9");  // "cafe"
string ascii = Text::UTF8::strip_diacritics(utf8_text, '?');  // custom fallback
```

### CSV Parsing

```cpp
Text::CSV csv;            // comma-separated
Text::CSV tsv('\t');      // tab-separated

// Parse single line
vector<string> fields;
csv.read_line("name,\"Smith, John\",42", fields);
// {"name", "Smith, John", "42"}

// Parse multiline text
vector<vector<string>> data;
csv.read("name,age\nAlice,30\nBob,25", data);
// data[0] = {"name", "age"}, data[1] = {"Alice", "30"}, ...

// Skip header row
csv.read(text, data, true);
```

## API Reference

### Free Functions

| Function | File | Description |
|----------|------|-------------|
| `strip_blank_lines(text)` | ws.cc | Strip single leading/trailing blank lines |
| `condense_blank_lines(text)` | ws.cc | Remove edge blanks, condense internal |
| `get_common_indent(text)` | ws.cc | Min leading whitespace |
| `remove_indent(text, n)` | ws.cc | Remove up to n spaces indent |
| `canonicalise_space(text)` | ws.cc | Trim + collapse whitespace |
| `remove_space(text)` | ws.cc | Remove all whitespace |
| `remove_word(text)` | ws.cc | Pop first word |
| `remove_line(text)` | ws.cc | Pop first line |
| `split_words(text)` | ws.cc | Split into words |
| `split_lines(text, rm_blanks)` | ws.cc | Split into lines |
| `split(text, delim, canon, max)` | split.cc | Split by delimiter |
| `pattern_match(pat, text, ...)` | pattern.cc | Glob matching with optional captures |
| `subst(text, old, rep)` | subst.cc | Global string replace |
| `tolower(text)` | case.cc | Lower-case |
| `toupper(text)` | case.cc | Upper-case |
| `itos`, `stoi`, `i64tos`, `stoi64` | convert.cc | Integer conversions |
| `ifixtos`, `stoifix` | convert.cc | Fixed-point conversions |
| `ftos`, `stof` | convert.cc | Float conversions |
| `stob` | convert.cc | String to bool |
| `itox`, `xtoi`, `i64tox`, `xtoi64` | convert.cc | Hex conversions |
| `btox`, `xtob` | convert.cc | Binary-hex conversions |

### Classes

| Class | Key Methods |
|-------|-------------|
| `Base64` | `encode(...)`, `decode(...)`, `binary_length(...)` |
| `Base64URL` | `encode(string)` (URL-safe, no padding) |
| `Base58` | `encode(vector<byte>)`, `decode(string, vector<byte>)` |
| `Base36` | `encode(uint64_t)`, `decode(string, uint64_t)` (static) |
| `Base16Alpha` | `encode(uint64_t)`, `decode(string, uint64_t)` (static) |
| `Bech32` | `encode(vector<byte>)`, `decode(...)`, `decode_as_5_bit(...)` (static) |
| `UTF8` | `append(...)`, `encode(...)`, `decode(...)`, `strip_diacritics(...)` (static) |
| `CSV` | `read_line(line, vars)`, `read(text, data, skip_header)` |

## Build

```
NAME    = ot-text
TYPE    = lib
DEPENDS = ot-gen
```

## License

Copyright (c) 2003 Paul Clark. MIT License.
