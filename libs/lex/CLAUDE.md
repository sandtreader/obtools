# CLAUDE.md - ObTools::Lex Library

## Overview

`ObTools::Lex` is a simple lexical analyser/tokeniser for JSON and C-like languages. Lives under `namespace ObTools::Lex`.

**Header:** `ot-lex.h`
**Dependencies:** `ot-text`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Token` | Token with type and value |
| `Analyser` | Main lexical analyser |
| `Exception` | Parse error |

## Token Types

```cpp
enum { UNKNOWN, END, NAME, NUMBER, STRING, SYMBOL };
```

## Analyser

```cpp
Analyser(istream& input);
void add_symbol(const string& sym);           // register symbol
void add_line_comment_symbol(const string& sym);
void disallow_alphanum_names();
void allow_dashed_names();
Token read_token();
void put_back(const Token& t);                // lookahead
```
