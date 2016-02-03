//==========================================================================
// ObTools::JSON: ot-json.h
//
// Public definitions for ObTools::JSON
//
// JSON format reader/writer
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_JSON_H
#define __OBTOOLS_JSON_H

#include <string>
#include <vector>
#include <map>
#include "ot-lex.h"
#include "ot-text.h"

namespace ObTools { namespace JSON {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// JSON value
struct Value
{
  enum Type
  {
    NULL_,   // Not set
    NUMBER,  // Floating point number
    STRING,  // Quoted string
    OBJECT,  // Object with properties
    ARRAY,   // Array of values
    TRUE,    // Boolean true
    FALSE    // Boolean value
  } type;

  double n;
  string s;
  map <string, Value> o;
  vector<Value> a;

  // Constructors
  Value(): type(NULL_), n(0) {}
  Value(Type _type): type(_type), n(0) {}
  Value(double _n): type(NUMBER), n(_n) {}
  Value(const string& _s): type(STRING), n(0), s(_s) {}
};

//==========================================================================
// Parser exception
struct Exception
{
  string error;
  Exception(const string& _error): error(_error) {}
};

//==========================================================================
// JSON parser
class Parser
{
  Lex::Analyser lex;

public:
  //------------------------------------------------------------------------
  // Constructor on an istream
  Parser(istream& _input);

  //------------------------------------------------------------------------
  // Read a value
  Value read_value() throw(Exception);
};

//==========================================================================
}} // namespaces

#endif // !__OBTOOLS_EXPR_H
