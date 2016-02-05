//==========================================================================
// ObTools::Lex: ot-lex.h
//
// Public definitions for ObTools::Lex
//
// Simple lexical analyser / tokeniser for JSON and other C-like languages
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_LEX_H
#define __OBTOOLS_LEX_H

#include <string>
#include <iostream>
#include <cstdio>
#include <list>

namespace ObTools { namespace Lex {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Token
struct Token
{
  enum Type
  {
    UNKNOWN, // Not set
    END,     // End of input
    NAME,    // Any alphanumeric/_ name
    NUMBER,  // Any integer / float beginning with digit
    STRING,  // Quoted ("") string
    SYMBOL   // Symbol from symbol dictionary
  } type;

  string value;

  // Constructors
  Token(): type(UNKNOWN) {}
  Token(Type _type): type(_type) {}
  Token(Type _type, const string& _value): type(_type), value(_value) {}
};

//==========================================================================
// Lex exception
struct Exception
{
  string error;
  Exception(const string& _error): error(_error) {}
};

//==========================================================================
// Lexical analyser
class Analyser
{
  istream& input;
  list<string> symbols;
  Token pending_token;

  // Tidied up get/peek - returns 0 for EOF
  char get() const { char c; return input.get(c)?c:0; }
  char peek() const
  { int c = input.peek(); return c==EOF?0:static_cast<char>(c); }

  // Internals
  Token read_name_token(char c) throw (Exception);
  Token read_number_token(char c) throw (Exception);
  Token read_string_token() throw (Exception);
  Token read_symbol_token(char c) throw (Exception);

public:
  //------------------------------------------------------------------------
  // Constructor on an istream
  Analyser(istream& _input): input(_input) {}

  //------------------------------------------------------------------------
  // Add a symbol - symbols are greedy matched
  void add_symbol(const string& symbol) { symbols.push_back(symbol); }

  //------------------------------------------------------------------------
  // Put back a token to be read next time (lookahead)
  void put_back(const Token& token) { pending_token = token; }

  //------------------------------------------------------------------------
  // Read a token from the input
  Token read_token() throw (Exception);
};

//==========================================================================
}} // namespaces

#endif // !__OBTOOLS_EXPR_H
