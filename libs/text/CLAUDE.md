# CLAUDE.md - ObTools::Text Library

## Overview

`ObTools::Text` is a string manipulation and encoding utility library providing whitespace handling, pattern matching, type conversions, Base64/Base58/Base36/Bech32 encoding, UTF-8 support, CSV parsing, and string substitution. Lives under `namespace ObTools::Text`.

**Header:** `ot-text.h`
**Dependencies:** `ot-gen`

## Key Components

| Component | Purpose |
|-----------|---------|
| Free functions | Whitespace, splitting, pattern matching, substitution, case conversion |
| Conversion functions | `itos`, `stoi`, `ftos`, `stof`, `stob`, `itox`, `xtoi`, `btox`, `xtob`, etc. |
| `Base64` / `Base64URL` | Base64 encoding/decoding (RFC and URL-safe variants) |
| `Base58` | Base58 encoding/decoding (Bitcoin-style) |
| `Base36` | Base36 encoding/decoding |
| `Base16Alpha` | Vowel-free safe alphabet encoding |
| `Bech32` | Bech32 encoding/decoding |
| `UTF8` | UTF-8 encode/decode, diacritics stripping |
| `CSV` | CSV line/file parsing |

## Whitespace Functions (ws.cc)

```cpp
string strip_blank_lines(const string& text);        // strip single leading/trailing blank lines
string condense_blank_lines(const string& text);     // remove edge blanks, condense internal
int get_common_indent(const string& text);            // min leading whitespace (tabs=8)
string remove_indent(const string& text, int indent); // remove up to indent spaces
string canonicalise_space(const string& text);        // trim + collapse internal ws to single space
string remove_space(const string& text);              // remove all whitespace
string remove_word(string& text);                     // pop first word from canonical text
string remove_line(string& text);                     // pop first line
vector<string> split_words(const string& text);       // split into words (canonicalises first)
vector<string> split_lines(const string& text, bool remove_blanks = false);
```

## Splitting (split.cc)

```cpp
vector<string> split(const string& text, char delim=',',
                     bool canonicalise=true, int max=0);
```

## Pattern Matching (pattern.cc)

Unix glob-style: `*` any chars, `?` one char, `[abc]` set, `[!abc]` negated set, `\` escape.

```cpp
bool pattern_match(const string& pattern, const string& text,
                   vector<string>& matches, bool cased=true);
bool pattern_match(const string& pattern, const string& text, bool cased=true);
```

## Substitution (subst.cc)

```cpp
string subst(string text, const string& old, const string& rep);  // s/old/rep/g
```

## Case Conversion (case.cc)

```cpp
string tolower(const string& text);
string toupper(const string& text);
```

## Type Conversions (convert.cc)

| Function | Description |
|----------|-------------|
| `itos(int)` / `stoi(string)` | int <-> string |
| `i64tos(uint64_t)` / `stoi64(string)` | 64-bit int <-> string |
| `ifixtos(int, dp)` / `stoifix(string, dp)` | fixed-point int <-> string |
| `ftos(double, width, prec, zero_pad)` / `stof(string)` | double <-> string |
| `stob(string)` | string -> bool (T/t/Y/y/1 = true) |
| `itox(unsigned)` / `xtoi(string)` | int <-> hex string |
| `i64tox(uint64_t)` / `xtoi64(string)` | 64-bit <-> hex string |
| `btox(data, len)` / `btox(string)` / `btox(vector<byte>)` | binary -> hex |
| `xtob(hex, data, max)` / `xtob(hex)` | hex -> binary |

## Base64

```cpp
Base64(char pad='=', char extra_62='+', char extra_63='/');
string encode(const unsigned char *block, size_t length, int split=76, const string& line_end="\r\n");
string encode(const string& binary, int split=76, const string& line_end="\r\n");
string encode(const vector<byte>& binary, int split=76, const string& line_end="\r\n");
string encode(uint64_t n);
size_t decode(const string& base64, unsigned char *block, size_t max_length);
bool decode(const string& base64, string& binary);
bool decode(const string& base64, vector<byte>& binary);
bool decode(const string& base64, uint64_t& n);
size_t binary_length(const string& base64);
```

`Base64URL` - URL-safe variant: no padding, uses `-` and `_`. `encode(string)` with no line splits.

## Base58

```cpp
Base58();                           // standard alphabet
Base58(const char *alphabet);       // custom 58-char alphabet
string encode(const vector<byte>& binary);
bool decode(const string& base58, vector<byte>& binary);
```

## Base36 / Base16Alpha / Bech32

All use static methods:

```cpp
// Base36
static string Base36::encode(uint64_t n);
static bool Base36::decode(const string& base36, uint64_t& n);

// Base16Alpha (vowel-free: "bcdg hjkl mpqr svwz")
static string Base16Alpha::encode(uint64_t n);
static bool Base16Alpha::decode(const string& base16, uint64_t& n);

// Bech32
static string Bech32::encode(const vector<byte>& binary);
static bool Bech32::decode(const string& bech32, vector<byte>& binary);
static bool Bech32::decode_as_5_bit(const string& bech32, vector<uint8_t>& binary);
```

## UTF8

```cpp
static void UTF8::append(string& utf8, wchar_t unicode);
static string UTF8::encode(const vector<wchar_t>& unicode);
static string UTF8::encode(const wchar_t *unicode);
static string UTF8::encode(const string& isolatin1);            // ISO-Latin1 -> UTF-8
static void UTF8::decode(const string& utf8, vector<wchar_t>& unicode);
static string UTF8::strip_diacritics(const string& utf8, char fallback='_');
```

## CSV

```cpp
CSV(char sep=',');
void read_line(const string& line, vector<string>& vars);
void read(const string& text, vector<vector<string>>& data, bool skip_header=false);
```

## File Layout

```
ot-text.h               - Public header
base16alpha.cc           - Base16Alpha encoder/decoder
base36.cc                - Base36 encoder/decoder
base58.cc                - Base58 encoder/decoder
base64.cc                - Base64 encoder/decoder
bech32.cc                - Bech32 encoder/decoder
case.cc                  - Case conversion
convert.cc               - Type conversions
csv.cc                   - CSV reader
pattern.cc               - Glob pattern matching
split.cc                 - String splitting
subst.cc                 - String substitution
utf8.cc                  - UTF-8 encoding
ws.cc                    - Whitespace handling
test-base58.cc           - Base58 tests (gtest)
test-base64.cc           - Base64/Base64URL tests
test-bech32.cc           - Bech32 tests
test-convert.cc          - Conversion tests
test-csv.cc              - CSV tests
test-pattern.cc          - Pattern matching tests
test-utf8.cc             - UTF-8 tests
test-ws-gtest.cc         - Whitespace tests
Tupfile                  - Build configuration
```
