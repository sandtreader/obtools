//==========================================================================
// ObTools::AMF: ot-amf.h
//
// Public definitions for AMF (Adobe Message Format) library
//
// Copyright (c) 2012 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_AMF_H
#define __OBTOOLS_AMF_H

#include <vector>
#include <string>
#include <map>
#include <stdint.h>
#include "ot-chan.h"

namespace ObTools { namespace AMF {

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Encoding format
enum Format
{
  FORMAT_AMF0,
  FORMAT_AMF3
};

//==========================================================================
// Exceptions
class Exception
{
public:
  string text;
  Exception(): text("") {}
  Exception(const string& t): text(t) {}
};

//==========================================================================
// AMF Value
struct Value
{
  enum Type
  {
    UNDEFINED  = 0,  // Numbered as per AMF-3
    NULLV      = 1,  // Note not NULL, because that's a macro :-(
    FALSE      = 2,
    TRUE       = 3,
    INTEGER    = 4,
    DOUBLE     = 5,
    STRING     = 6,
    XML_DOC    = 7,
    DATE       = 8,
    ARRAY      = 9,
    OBJECT     = 10,
    XML        = 11,
    BYTE_ARRAY = 12
  } type;

  typedef int32_t integer_t;

  // Not all in union because we can't put strings in it
  union
  {
    integer_t n; // INTEGER
    double d;    // DOUBLE, DATE
  };

  string text;  // STRING, XML-DOC, XML, BYTE_ARRAY

  // Array split into dense (indexed) and associative parts
  // Associative also used for object - traits ignored
  vector<Value> dense_array;
  map<string, Value> assoc_array;

  //------------------------------------------------------------------------
  // Constructors
  Value(): type(UNDEFINED) {}
  Value(Type _type): type(_type) {}
  Value(Type _type, integer_t _n): type(_type), n(_n) {}
  Value(Type _type, double _d): type(_type), d(_d) {}
  Value(Type _type, const string& _t): type(_type), text(_t) {}

  //------------------------------------------------------------------------
  // Comparator
  bool operator==(const Value& o) const;
  bool operator!=(const Value& o) const { return !(*this ==o); }

  //------------------------------------------------------------------------
  // Get a value from an associative array/object
  Value get(const string& name) throw (Exception);

  //------------------------------------------------------------------------
  // Get a value from an associative array/object, checking for a specific type
  Value get(const string& name, Type type, const string& type_name)
    throw (Exception);

  //------------------------------------------------------------------------
  // Get an integer value from an associative array/object
  integer_t get_integer(const string& name) throw (Exception)
  { return get(name, INTEGER, "INTEGER").n; }

  //------------------------------------------------------------------------
  // Get a double value from an associative array/object
  double get_double(const string& name) throw (Exception)
  { return get(name, DOUBLE, "DOUBLE").d; }

  //------------------------------------------------------------------------
  // Get a string value from an associative array/object
  string get_string(const string& name) throw (Exception)
  { return get(name, STRING, "STRING").text; }

  //------------------------------------------------------------------------
  // Get a boolean value from an associative array/object
  bool get_boolean(const string& name) throw (Exception);

  //------------------------------------------------------------------------
  // Add an indexed array entry to the dense array
  void add(const Value& value)
  { dense_array.push_back(value); }

  //------------------------------------------------------------------------
  // Set a named value in the assoc_array
  void set(const string& name, const Value& value)
  { assoc_array[name] = value; }

  //------------------------------------------------------------------------
  // Read from a channel in the given format
  void read(Channel::Reader& chan, Format format)
    throw (Exception, Channel::Error);

  //------------------------------------------------------------------------
  // Write to a channel in the given format
  void write(Channel::Writer& chan, Format format)
    throw (Exception, Channel::Error);

  //------------------------------------------------------------------------
  // Pretty print the message
  ostream& log(ostream& sout, const string& indent="") const;
};

//--------------------------------------------------------------------------
// << operator to write a Value to ostream
ostream& operator<<(ostream& s, const Value& c);

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_AMF_H



