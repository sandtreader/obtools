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
class Value
{
  void write_string_to(ostream& out) const;
  void write_object_to(ostream& out, bool pretty, int indent) const;
  void write_array_to(ostream& out, bool pretty, int indent) const;

public:
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
  Value(const char *_s): type(STRING), n(0), s(_s) {}

  //------------------------------------------------------------------------
  // Set a property on an object value
  void set(const string& name, const Value& v) { o[name] = v; }

  //------------------------------------------------------------------------
  // Add an element to an array value
  void add(const Value& v) { a.push_back(v); }

  //------------------------------------------------------------------------
  // Write the value to the given stream
  // Set 'pretty' for multi-line, indented pretty-print, clear for optimal
  void write_to(ostream& s, bool pretty=false, int indent=0) const;
};

//------------------------------------------------------------------------
// >> operator to write to ostream
ostream& operator<<(ostream& s, const Value& v);

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
