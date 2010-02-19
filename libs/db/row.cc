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

//------------------------------------------------------------------------
//Get value of field of given name, or default if not found
string Row::get(string fieldname, const string& def) const
{
  map<string, FieldValue>::const_iterator p=fields.find(fieldname);
  if (p!=fields.end())
    return p->second.as_string();
  else
    return def;
}

//------------------------------------------------------------------------
//Get integer value of field of given name, or default if not found
int Row::get_int(string fieldname, int def) const
{
  map<string, FieldValue>::const_iterator p=fields.find(fieldname);
  if (p!=fields.end())
    return p->second.as_int();
  else
    return def;
}

//------------------------------------------------------------------------
//Get 64-bit value of field of given name, or default if not found
uint64_t Row::get_int64(string fieldname, uint64_t def) const
{
  map<string, FieldValue>::const_iterator p=fields.find(fieldname);
  if (p!=fields.end())
    return p->second.as_int64();
  else
    return def;
}

//------------------------------------------------------------------------
//Get boolean value of field of given name, or default if not found
bool Row::get_bool(string fieldname, bool def) const
{
  map<string, FieldValue>::const_iterator p=fields.find(fieldname);
  if (p!=fields.end())
    return p->second.as_bool();
  else
    return def;
}

//------------------------------------------------------------------------
//Get real value of field of given name, or default if not found
double Row::get_real(string fieldname, double def) const
{
  map<string, FieldValue>::const_iterator p=fields.find(fieldname);
  if (p!=fields.end())
    return p->second.as_real();
  else
    return def;
}

//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
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

//------------------------------------------------------------------------
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

}} // namespaces
