//==========================================================================
// ObTools::Expression: ot-expr.h
//
// Public definitions for ObTools::Expression
// Basic expression parser with the following grammar:
//
// EXPR:   PRED ([ & && | || ] PRED)+
// PRED:   SIDE [ = == < > <= >= <> !=] SIDE
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

namespace ObTools { namespace Expression {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Token
struct Token
{
  enum Type
  {
    UNKNOWN, // Not recognised
    EOT,     // End of text
    NUMBER,  // Any float
    NAME,    // Any variable name

    MUL,     // *
    DIV,     // /

    PLUS,    // +
    MINUS,   // -

    AND,     // & or &&
    OR,      // | or ||
    NOT,     // !

    EQ,      // = or ==
    LT,      // <
    GT,      // >
    LTEQ,    // <=
    GTEQ,    // >=
    NE,      // <> or !=

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
// Tokeniser
class Tokeniser
{
  string input;
  string::iterator it;
  ostream& serr;

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
  // Constructor on string input, with error channel
  Tokeniser(const string& _input, ostream& _serr):
    input(_input), it(input.begin()), serr(_serr) {}

  //------------------------------------------------------------------------
  // Constructor with only error channel - use reset() to set input
  Tokeniser(ostream& _serr): serr(_serr) {}

  //------------------------------------------------------------------------
  // Reset input to given string
  void reset(const string& _input)
  { input = _input;  it = input.begin(); }

  //------------------------------------------------------------------------
  // Read a token from the input
  Token read_token();
};

//==========================================================================
// Evaluator
// Override get_name_value() for variable binding
class Evaluator
{
  ostream& serr;
  Tokeniser tokeniser;
  Token token;

  void next();
  double read_factor();
  double read_term();
  double read_side();
  double read_predicate();
  double read_expression();

protected:
  //------------------------------------------------------------------------
  // Get value for a name in the expression
  // By default just errors and returns 0 - override if variables required
  virtual double get_value_for_name(const string& name);

public:
  //------------------------------------------------------------------------
  // Constructor with error channel
  Evaluator(ostream& _serr): serr(_serr), tokeniser(serr) {}

  //------------------------------------------------------------------------
  // Evaluate an expression
  double evaluate(const string& expr);
};

//==========================================================================
}} // namespaces

#endif // !__OBTOOLS_EXPR_H
