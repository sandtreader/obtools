# CLAUDE.md - ObTools::CPPT Library

## Overview

`ObTools::CPPT` is a C++ template processor for code generation — embeds C++ code and expressions within template text using configurable tags. Lives under `namespace ObTools::CPPT`.

**Header:** `ot-cppt.h`
**Dependencies:** none

## Key Classes

| Class | Purpose |
|-------|---------|
| `Processor` | Main template processor (input -> C++ output) |
| `TokenRecogniser` | Multi-character token recogniser for tag parsing |
| `Tags` | Configuration for template delimiters |

## Processor States

```cpp
enum ProcessorState { PS_NORMAL, PS_CODE, PS_EXPR, PS_COMMENT };
```

## Tags

```cpp
struct Tags {
  string start_code, end_code;       // code block delimiters
  string start_expr, end_expr;       // expression delimiters
  string start_comment, end_comment; // comment delimiters
};
```

## Processor

```cpp
Processor(istream& in, ostream& out, const Tags& tags,
          const string& streamname = "cout");
void process();  // transform template to C++ code
```

## TokenRecogniser

```cpp
TokenRecogniser();
void add_token(const string& tok);
bool process_char(char c, TokenState& state);
string get_token();
```

## How It Works

Template text is converted to C++ `cout <<` statements. Code blocks pass through as-is. Expressions are wrapped in output statements.

```
Hello <?= name ?>!   →   cout << "Hello " << name << "!" << endl;
<? if (x) { ?>       →   if (x) {
  Yes!                →     cout << "  Yes!" << endl;
<? } ?>               →   }
```
