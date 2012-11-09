//==========================================================================
// ObTools::DB: field-value.cc
//
// Implementation of result row field value
//
// Copyright (c) 2003-2007 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-db.h"
#include "ot-text.h"
#include <stdlib.h>
#include <sstream>

namespace ObTools { namespace DB {

//------------------------------------------------------------------------
// Convert a DB boolean represented as a string to a bool
bool db_str_to_bool(const string& value)
{
  if (value.empty())
    return false;

  switch(value[0])
  {
    case 'T': case 't':
    case 'Y': case 'y':
    case '1':
      return true;

    default:
    return false;
  }
}
//------------------------------------------------------------------------
// Constructor from string but convert to specified type
FieldValue::FieldValue(const string& value, FieldType _type):
  type(_type)
{
  switch (type)
  {
    case NULLTYPE:
      break;

    case STRING:
      str_val = value;
      break;

    case INT:
      int_val = Text::stoi(value);
      break;

    case INT64:
      int64_val = Text::stoi64(value);
      break;

    case BOOL:
      bool_val = db_str_to_bool(value);
      break;

    case REAL:
      real_val = Text::stof(value);
      break;
  }
}

//------------------------------------------------------------------------
// Get value as string
string FieldValue::as_string() const
{
  switch (type)
  {
    case NULLTYPE:
      return "NULL";

    case STRING:
      return str_val;

    case INT:
      return Text::itos(int_val);

    case INT64:
      return Text::i64tos(int64_val);

    case BOOL:
      return bool_val ? "1" : "0";

    case REAL:
      return Text::ftos(real_val);

    default:
      return "";
  }
}

//------------------------------------------------------------------------
// Get value as int
int FieldValue::as_int() const
{
  switch (type)
  {
    case INT:
      return int_val;

    case INT64:
      return int64_val;

    case BOOL:
      return bool_val;

    case REAL:
      return (int)real_val;

    case NULLTYPE:
      return 0;

    case STRING:
      return Text::stoi(str_val);

    default:
      return 0;
  }
}

//------------------------------------------------------------------------
// Get value as int64
uint64_t FieldValue::as_int64() const
{
  switch (type)
  {
    case INT:
      return int_val;

    case INT64:
      return int64_val;

    case BOOL:
      return bool_val;

    case REAL:
      return (uint64_t)real_val;

    case NULLTYPE:
      return 0;

    case STRING:
      return Text::stoi64(str_val);

    default:
      return 0;
  }
}

//------------------------------------------------------------------------
// Get value as bool
bool FieldValue::as_bool() const
{
  switch (type)
  {
    case INT:
      return int_val;

    case INT64:
      return int64_val;

    case BOOL:
      return bool_val;

    case REAL:
      return real_val;

    case NULLTYPE:
      return false;

    case STRING:
      return db_str_to_bool(str_val);

    default:
      return 0;
  }
}

//------------------------------------------------------------------------
// Get value as real
double FieldValue::as_real() const
{
  switch (type)
  {
    case INT:
      return int_val;

    case INT64:
      return int64_val;

    case BOOL:
      return bool_val;

    case REAL:
      return real_val;

    case NULLTYPE:
      return false;

    case STRING:
      return Text::stof(str_val);

    default:
      return 0;
  }
}

//------------------------------------------------------------------------
// Escape a string, doubling single quotes and backslashes
string FieldValue::escape(const string& s)
{
  string s2 = Text::subst(s, "\\", "\\\\");
  return Text::subst(s2, "'", "''");
}

//------------------------------------------------------------------------
// Unescape a string, singling double quotes and backslashes
string FieldValue::unescape(const string& s)
{
  string s2 = Text::subst(s, "\\\\", "\\");
  return Text::subst(s2, "''", "'");
}

}} // namespaces
