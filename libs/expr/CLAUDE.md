# CLAUDE.md - ObTools::Expression Library

## Overview

`ObTools::Expression` is a mathematical and logical expression parser/evaluator with variable support. Lives under `namespace ObTools::Expression`.

**Header:** `ot-expr.h`
**Dependencies:** `ot-misc`

## Key Classes

| Class | Purpose |
|-------|---------|
| `Tokeniser` | Lexical analyser for expressions |
| `Evaluator` | Expression evaluator (subclass for variables) |
| `PropertyListEvaluator` | Evaluator with `Misc::PropertyList` variable binding |
| `Token` | Token with type, name, and numeric value |
| `Exception` | Parse/evaluation error |

## Grammar

```
EXPR:   PRED ([ && || ] PRED)+
PRED:   SIDE [ == < > <= >= != ] SIDE
SIDE:   TERM ([ + - ] TERM)+
TERM:   FACTOR ([ * / ] FACTOR)+
FACTOR: [ ! - ] [ number | variable | (EXPR) ]
```

## Token Types

```cpp
enum Token::Type {
  UNKNOWN, EOT, NUMBER, NAME,
  MUL, DIV, PLUS, MINUS,
  AND, OR, NOT,
  EQ, LT, GT, LTEQ, GTEQ, NE,
  LPAR, RPAR
};
```

## Evaluator

```cpp
Evaluator();
double evaluate(const string& expr);              // parse and evaluate
virtual double get_value_for_name(const string& name);  // override for variables
```

## PropertyListEvaluator

```cpp
PropertyListEvaluator(Misc::PropertyList& _vars);
// Looks up variable values from PropertyList
```

## Supported Expressions

```
1234, 1234.567          // numbers
3+2, 3*2, 3/2           // arithmetic
1&&1, 0||1, !0          // logical
2==2, 3!=2, 2<3         // comparison
2+2*2<=6.0 && 3+2==5   // complex
foo, foo+bar            // variables
```
