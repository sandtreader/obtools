# ObTools::Expression

Mathematical and logical expression parser/evaluator with variable support.

Part of the [ObTools](https://github.com/sandtreader/obtools) library collection.

## Quick Start

```cpp
#include "ot-expr.h"
using namespace ObTools;

// Simple evaluation
Expression::Evaluator eval;
double result = eval.evaluate("2 + 3 * 4");  // 14.0

// Comparisons and logic
result = eval.evaluate("2 < 3 && 4 >= 4");   // 1.0 (true)

// With variables from PropertyList
Misc::PropertyList vars;
vars.add("x", "10");
vars.add("y", "20");

Expression::PropertyListEvaluator peval(vars);
result = peval.evaluate("x + y");             // 30.0
result = peval.evaluate("x < y && y > 15");   // 1.0 (true)
```

### Error Handling

```cpp
try
{
  eval.evaluate("2 + + 3");
}
catch (const Expression::Exception& e)
{
  cerr << "Parse error: " << e.error << endl;
}
```

## Build

```
NAME    = ot-expr
TYPE    = lib
DEPENDS = ot-misc
```

## License

Copyright (c) 2016 Paul Clark. MIT License.
