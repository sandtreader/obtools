//==========================================================================
// ObTools::DB: row.cc
//
// Implementation of result row
//
// Copyright (c) 2003-2007 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-db.h"
#include "ot-text.h"
#include <stdlib.h>
#include <sstream>

namespace ObTools { namespace DB {

//--------------------------------------------------------------------------
// Get value of field of given name, or default if not found
string Row::get(const string& fieldname, const string& def) const
{
  map<string, FieldValue>::const_iterator p=fields.find(fieldname);
  if (p!=fields.end())
    return p->second.as_string();
  else
    return def;
}

//--------------------------------------------------------------------------
// Get integer value of field of given name, or default if not found
int Row::get_int(const string& fieldname, int def) const
{
  map<string, FieldValue>::const_iterator p=fields.find(fieldname);
  if (p!=fields.end())
    return p->second.as_int();
  else
    return def;
}

//--------------------------------------------------------------------------
// Get 64-bit value of field of given name, or default if not found
uint64_t Row::get_int64(const string& fieldname, uint64_t def) const
{
  map<string, FieldValue>::const_iterator p=fields.find(fieldname);
  if (p!=fields.end())
    return p->second.as_int64();
  else
    return def;
}

//--------------------------------------------------------------------------
// Get boolean value of field of given name, or default if not found
bool Row::get_bool(const string& fieldname, bool def) const
{
  map<string, FieldValue>::const_iterator p=fields.find(fieldname);
  if (p!=fields.end())
    return p->second.as_bool();
  else
    return def;
}

//--------------------------------------------------------------------------
// Get real value of field of given name, or default if not found
double Row::get_real(const string& fieldname, double def) const
{
  map<string, FieldValue>::const_iterator p=fields.find(fieldname);
  if (p!=fields.end())
    return p->second.as_real();
  else
    return def;
}

//--------------------------------------------------------------------------
// Get string with field names in order, separated by commas and spaces
string Row::get_fields() const
{
  string result;
  for(map<string, FieldValue>::const_iterator p = fields.begin();
      p!=fields.end(); ++p)
  {
    if (!result.empty()) result += ", ";
    result += p->first;
  }

  return result;
}

//--------------------------------------------------------------------------
// Get fields that are *not* in a suppressed fields row
string Row::get_fields_not_in(const Row& suppressed_fields) const
{
  string result;
  for(const auto& p: fields)
  {
    if (!suppressed_fields.has(p.first))
    {
      if (!result.empty()) result += ", ";
      result += p.first;
    }
  }

  return result;
}

//--------------------------------------------------------------------------
// Get string with field values in order, separated by commas and spaces,
// with assigment back to VALUES(name) - e.g. x=VALUES(x), y=VALUES(y)
// (e.g. for INSERT .. ON DUPLICATE KEY UPDATE)
string Row::get_fields_set_to_own_values() const
{
  string result;
  for(map<string, FieldValue>::const_iterator p = fields.begin();
      p!=fields.end(); ++p)
  {
    if (!result.empty()) result += ", ";
    result += p->first + " = VALUES(" + p->first + ")";
  }

  return result;
}

//--------------------------------------------------------------------------
// Get string with field values in order, separated by commas and spaces,
// each delimited with single quotes (e.g. for INSERT)
string Row::get_escaped_values() const
{
  string result;
  for(map<string, FieldValue>::const_iterator p = fields.begin();
      p!=fields.end(); ++p)
  {
    if (!result.empty()) result += ", ";
    result += p->second.as_quoted_string();
  }

  return result;
}

//--------------------------------------------------------------------------
// Get string with field names and values in order with '=',
// separated by commas and spaces, values delimited with single quotes
// (e.g. for UPDATE)
string Row::get_escaped_assignments() const
{
  string result;
  for(map<string, FieldValue>::const_iterator p = fields.begin();
      p!=fields.end(); ++p)
  {
    if (!result.empty()) result += ", ";
    result += p->first + " = " + p->second.as_quoted_string();
  }

  return result;
}

//--------------------------------------------------------------------------
// Get string with field names and values in order with '=',
// separated by commas and spaces, values delimited with single quotes
// limited by another row (e.g. for ON CONFLICT ... DO UPDATE)
string Row::get_escaped_assignments_limited_by(const Row& limit) const
{
  string result;
  for(const auto& p: limit.fields)
  {
    auto q = fields.find(p.first);
    if (q != fields.end())
    {
      if (!result.empty()) result += ", ";
      result += p.first + " = " + q->second.as_quoted_string();
    }
  }

  return result;
}

//--------------------------------------------------------------------------
// Get string with field names and values in order with '=', separated
// by AND, values delimited with single quotes
// (e.g. for WHERE)
string Row::get_where_clause() const
{
  string result;
  for(map<string, FieldValue>::const_iterator p = fields.begin();
      p!=fields.end(); ++p)
  {
    if (!result.empty()) result += " AND ";
    if (p->second.is_null())
      result += p->first + " IS NULL";
    else
      result += p->first + " = " + p->second.as_quoted_string();
  }

  return result;
}

}} // namespaces
