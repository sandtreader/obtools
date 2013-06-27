//==========================================================================
// ObTools::Expression: ot-expr.h
//
// Public definitions for ObTools::Expression
// Basic expression parser with the following grammar, like C but without
// the bitwise operators and ternary operator.  Precedence is also slightly
// simplified
//
// EXPR:   PRED ([ && || ] PRED)+
// PRED:   SIDE [ == < > <= >= !=] SIDE
// SIDE:   TERM ([ + - ] TERM)+
// TERM:   FACTOR ([ * / ] FACTOR)+
// FACTOR: [ ! - ] [ number | variable | (EXPR)]
//
// Copyright (c) 2013 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_EXPR_H
#define __OBTOOLS_EXPR_H

#include <string>
#include <iostream>
#include "ot-misc.h"

namespace ObTools { namespace Expression {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Token
struct Token
{
  enum Type
  {
    UNKNOWN, // Not set
    EOT,     // End of text
    NUMBER,  // Any float
    NAME,    // Any variable name

    MUL,     // *
    DIV,     // /

    PLUS,    // +
    MINUS,   // -

    AND,     // &&
    OR,      // ||
    NOT,     // !

    EQ,      // ==
    LT,      // <
    GT,      // >
    LTEQ,    // <=
    GTEQ,    // >=
    NE,      // !=

    LPAR,    // (
    RPAR     // )
  } type;

  string name;
  double value;

  // Constructors
  Token(): type(UNKNOWN), value(0) {}
  Token(Type _type): type(_type), value(0) {}
  Token(Type _type, const string& _name): type(_type), name(_name), value(0) {}
  Token(Type _type, double _value): type(_type), value(_value) {}
};

//==========================================================================
// Expression exception
struct Exception
{
  string error;
  Exception(const string& _error): error(_error) {}
};

//==========================================================================
// Tokeniser
class Tokeniser
{
  string input;
  string::iterator it;

  //------------------------------------------------------------------------
  // Get next character or 0 for EOF and move forwards
  char getc() { return it==input.end()?0:*it++; }

  //------------------------------------------------------------------------
  // Get next character or 0 for EOF but don't move forwards
  char peekc() { return it==input.end()?0:*it; }

  //------------------------------------------------------------------------
  // Move forwards if not already at end
  void step() { if (it!=input.end()) ++it; }

public:
  //------------------------------------------------------------------------
  // Blank constructor - use reset() to set input
  Tokeniser() {}

  //------------------------------------------------------------------------
  // Constructor on string input
  Tokeniser(const string& _input) { reset(_input); }

  //------------------------------------------------------------------------
  // Reset input to given string
  void reset(const string& _input)
  { input = _input;  it = input.begin(); }

  //------------------------------------------------------------------------
  // Read a token from the input
  Token read_token() throw (Exception);
};

//==========================================================================
// Evaluator
// Override get_name_value() for variable binding
class Evaluator
{
  Tokeniser tokeniser;
  Token token;

  void next() throw (Exception);
  double read_factor() throw (Exception);
  double read_term() throw (Exception);
  double read_side() throw (Exception);
  double read_predicate() throw (Exception);
  double read_expression() throw (Exception);

protected:
  //------------------------------------------------------------------------
  // Get value for a name in the expression
  // By default just errors and returns 0 - override if variables required
  virtual double get_value_for_name(const string& name) throw (Exception);

public:
  //------------------------------------------------------------------------
  // Constructor
  Evaluator() {}

  //------------------------------------------------------------------------
  // Evaluate an expression
  double evaluate(const string& expr) throw (Exception);

  //------------------------------------------------------------------------
  // Virtual destructor to keep compiler happy!
  virtual ~Evaluator() {}
};

//==========================================================================
// Property list evaluator - gets variable values from property list
class PropertyListEvaluator: public Evaluator
{
  Misc::PropertyList& vars;

  // Implementation of get value
  double get_value_for_name(const string& name) throw (Exception)
  {
    if (vars.has(name)) return vars.get_real(name);
    throw Exception(string("No such variable '")+name+"'");
  }

public:
  //------------------------------------------------------------------------
  // Constructor
  PropertyListEvaluator(Misc::PropertyList& _vars): vars(_vars) {}
};

//==========================================================================
}} // namespaces

#endif // !__OBTOOLS_EXPR_H
