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
  static Value none;

  enum Type
  {
    UNSET,   // Not set
    NULL_,   // null
    NUMBER,  // Floating point number
    INTEGER, // Integer number - added for precision
    STRING,  // Quoted string
    OBJECT,  // Object with properties
    ARRAY,   // Array of values
    TRUE,    // Boolean true
    FALSE    // Boolean value
  } type;

  double f;
  uint64_t n;
  string s;
  map <string, Value> o;
  vector<Value> a;

  // Constructors
  Value(): type(UNSET), n(0) {}
  Value(Type _type): type(_type), n(0) {}
  Value(Type, double _f): type(NUMBER), f(_f) {} // note type included so not..
  Value(uint64_t _n): type(INTEGER), n(_n) {}    // ..ambiguous with this
  Value(const string& _s): type(STRING), n(0), s(_s) {}

  //------------------------------------------------------------------------
  // Set a property on an object value
  void set(const string& name, const Value& v) { o[name] = v; }

  // Special explicit cases for int and string to remove ambiguity on 0
  void set(const string& name, uint64_t _n) { set(name, Value(_n)); }
  void set(const string& name, const string& _s) { set(name, Value(_s)); }

  //------------------------------------------------------------------------
  // Add an element to an array value
  void add(const Value& v) { a.push_back(v); }

  // Special cases for int, string as above
  void add(uint64_t _n) { add(Value(_n)); }
  void add(const string& _s) { add(Value(_s)); }

  //------------------------------------------------------------------------
  // Check whether a value is valid - NB FALSE, NULL and 0 are still valid!
  bool operator!() const { return type == UNSET; }

  //------------------------------------------------------------------------
  // Check whether a value is true - TRUE or non-zero INTEGER accepted
  bool is_true() const { return type == TRUE || (type == INTEGER && n); }

  //------------------------------------------------------------------------
  // Get a string value with the given default
  const string& str(const string& def="") { return (type==STRING)?s:def; }

  //------------------------------------------------------------------------
  // Get a value from the given object property
  // Returns Value(NULL) if this is not an object or property doesn't exist
  const Value& get(const string& property) const;

  // [] operator using the above
  const Value& operator[](const string& property) const
  { return get(property); }

  //------------------------------------------------------------------------
  // Get a value from the given array index
  // Returns Value::none if this is not an array or index doesn't exist
  const Value& get(unsigned int index) const;

  // [] operator using the above
  const Value& operator[](unsigned int index) const
  { return get(index); }

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

  Value read_rest_of_array();
  Value read_rest_of_object();

public:
  //------------------------------------------------------------------------
  // Constructor on an istream
  Parser(istream& _input);

  //------------------------------------------------------------------------
  // Read a value
  Value read_value();
};

//==========================================================================
}} // namespaces

#endif // !__OBTOOLS_JSON_H
