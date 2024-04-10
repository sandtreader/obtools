//==========================================================================
// ObTools::JSON: ot-json.h
//
// Public definitions for ObTools::JSON
//
// JSON format reader/writer
//
// Copyright (c) 2016-2024 Paul Clark.
//==========================================================================

#ifndef __OBTOOLS_JSON_H
#define __OBTOOLS_JSON_H

#include <string>
#include <vector>
#include <map>
#include "ot-lex.h"
#include "ot-text.h"
#include "ot-chan.h"

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
    UNDEFINED, // Not set
    NULL_,     // null
    NUMBER,    // Floating point number
    INTEGER,   // Integer number - added for precision
    STRING,    // Quoted string
    OBJECT,    // Object with properties
    ARRAY,     // Array of values
    TRUE_,     // Boolean true
    FALSE_,    // Boolean value

    // Additional types for CBOR
    BINARY,    // Binary data
    BREAK      // End of indefinite array - only used internally
  } type;

  double f = 0.0;
  int64_t n = 0;
  string s;               // Used for STRING and BINARY
  map <string, Value> o;
  vector<Value> a;

  // Constructors
  Value(): type(UNDEFINED) {}
  Value(Type _type): type(_type) {}
  Value(float _f): type(NUMBER), f(_f) {}
  Value(double _f): type(NUMBER), f(_f) {}
  Value(int32_t _n): type(INTEGER), n(_n) {}
  Value(uint32_t _n): type(INTEGER), n(_n) {}
  Value(int64_t _n): type(INTEGER), n(_n) {}
  Value(uint64_t _n): type(INTEGER), n(_n) {}
  Value(const string& _s): type(STRING), s(_s) {}
  Value(const char *_s): type(STRING), s(_s) {}
  Value(const vector<unsigned char>& data):
    type(BINARY), s(data.begin(), data.end()) {}

  //------------------------------------------------------------------------
  // Set a constructed property on an object value - returns the Value added
  Value& put(const string& name, const Value& v) { return (o[name] = v); }

  //------------------------------------------------------------------------
  // Set a property on an object value - returns *this for chaining
  Value& set(const string& name, const Value& v) { o[name] = v; return *this; }

  //------------------------------------------------------------------------
  // Add an element to an array value - returns the Value added
  Value& add(const Value& v) { a.push_back(v); return a.back(); }

  //------------------------------------------------------------------------
  // Check whether a value is valid - NB FALSE, NULL and 0 are still valid!
  bool operator!() const { return type == UNDEFINED; }

  //------------------------------------------------------------------------
  // Check whether a value is true - TRUE or non-zero INTEGER accepted
  bool is_true() const { return type == TRUE_ || (type == INTEGER && n); }

  //------------------------------------------------------------------------
  // Comparators
  bool operator==(const Value& v) const;
  bool operator!=(const Value& v) const { return !(*this == v); }

  //------------------------------------------------------------------------
  // Get a value from the given object property
  // Returns Value::none if this is not an object or property doesn't exist
  const Value& get(const string& property) const;
  Value& get(const string& property);

  // [] operator using the above
  const Value& operator[](const string& property) const
  { return get(property); }
  Value& operator[](const string& property)
  { return get(property); }

  //------------------------------------------------------------------------
  // Get a value from the given array index
  // Returns Value::none if this is not an array or index doesn't exist
  const Value& get(unsigned int index) const;
  Value& get(unsigned int index);

  // [] operator using the above
  const Value& operator[](unsigned int index) const
  { return get(index); }
  Value& operator[](unsigned int index)
  { return get(index); }

  //------------------------------------------------------------------------
  // Get the size of an array (if it is an array, otherwise 0)
  size_t size() const
  { return (type==ARRAY)?a.size():0; }

  //------------------------------------------------------------------------
  // Read as a string value with the given default
  string as_str(const string& def="") const { return (type==STRING)?s:def; }

  //------------------------------------------------------------------------
  // Read as an integer value with the given default
  // Will cast numeric strings
  int64_t as_int(int64_t def=0) const
  { return (type==INTEGER)?n:
      ((type==STRING)?Text::stoi(s):def); }

  //------------------------------------------------------------------------
  // Read as a float value with the given default - also promotes integers
  // Will cast numeric strings
  double as_float(double def=0.0) const
  { return (type==NUMBER)?f:
      ((type==INTEGER)?n:
       ((type==STRING)?Text::stof(s):def)); }

  //------------------------------------------------------------------------
  // Write the value to the given stream
  // Set 'pretty' for multi-line, indented pretty-print, clear for optimal
  void write_to(ostream& s, bool pretty=false, int indent=0) const;

  //------------------------------------------------------------------------
  // Output value as a string, with optional prettiness
  string str(bool pretty=false) const;

  //------------------------------------------------------------------------
  // Output value as a CBOR binary string
  string cbor() const;
};

//--------------------------------------------------------------------------
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
// CBOR generator
class CBORWriter
{
  Channel::Writer& writer;
  void write_int(uint64_t v, unsigned char top_bits);

public:
  //------------------------------------------------------------------------
  // Construct on a channel writer
  CBORWriter(Channel::Writer& _writer): writer(_writer) {}

  //------------------------------------------------------------------------
  // Output a JSON value as CBOR
  void encode(const Value& v);

  //------------------------------------------------------------------------
  // Open an indefinite array
  // Then continue to write any number of member values, and close it
  void open_indefinite_array();

  //------------------------------------------------------------------------
  // Close an indefinite array
  void close_indefinite_array();
};

//==========================================================================
// CBOR parser
class CBORReader
{
  Channel::Reader& reader;
  uint64_t read_int(uint8_t initial_byte);

public:
  //------------------------------------------------------------------------
  // Construct on a channel reader
  CBORReader(Channel::Reader& _reader): reader(_reader) {}

  //------------------------------------------------------------------------
  // Read and decode a single CBOR value
  JSON::Value decode();
};

//==========================================================================
}} // namespaces

#endif // !__OBTOOLS_JSON_H
