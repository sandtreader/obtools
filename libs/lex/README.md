# ObTools::Lex

Simple lexical analyser/tokeniser for JSON and C-like languages.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-lex.h"
using namespace ObTools;

istringstream input("name = 42;");
Lex::Analyser lex(input);
lex.add_symbol("=");
lex.add_symbol(";");

Lex::Token t = lex.read_token();  // NAME "name"
t = lex.read_token();              // SYMBOL "="
t = lex.read_token();              // NUMBER "42"
t = lex.read_token();              // SYMBOL ";"
```

## Build

```
NAME    = ot-lex
TYPE    = lib
DEPENDS = ot-text
```

## License

Copyright (c) 2016 Paul Clark. MIT License.
