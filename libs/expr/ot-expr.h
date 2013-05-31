//==========================================================================
// ObTools::Expression: ot-expr.h
//
// Public definitions for ObTools::Expression
// Basic expression parser
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
  // Get next character or 0 for EOF
  char getc() { return it==input.end()?0:*it++; }

  //------------------------------------------------------------------------
  // Replace a character
  void rewind() { if (it != input.begin() && it != input.end()) --it; }

public:
  //------------------------------------------------------------------------
  // Constructor on string input, with error channel
  Tokeniser(const string& _input, ostream& _serr):
    input(_input), it(input.begin()), serr(_serr) {}

  //------------------------------------------------------------------------
  // Read a token from the input
  Token read_token();
};

//==========================================================================
}} // namespaces

#endif // !__OBTOOLS_EXPR_H
