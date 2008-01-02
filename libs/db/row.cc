//==========================================================================
// ObTools::DB: row.cc
//
// Implementation of result row
//
// Copyright (c) 2003-2007 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-db.h"
#include "ot-text.h"
#include <sstream>

namespace ObTools { namespace DB {

//------------------------------------------------------------------------
// Add integer value to row
void Row::add(string fieldname, int value)
{
  fields[fieldname] = Text::itos(value);
}

//------------------------------------------------------------------------
// Add 64-bit integer value to row
void Row::add_int64(string fieldname, uint64_t value)
{
  fields[fieldname] = Text::i64tos(value);
}

//------------------------------------------------------------------------
// Add boolean value to row
void Row::add(string fieldname, bool value)
{
  // Make compatible with both Postgres and MySql
  fields[fieldname] = value?"1":"0";  
}

//------------------------------------------------------------------------
//Get value of field of given name, or default if not found
string Row::get(string fieldname, const string& def) const
{
  map<string,string>::const_iterator p=fields.find(fieldname);
  if (p!=fields.end())
    return p->second;
  else
    return def;
}

//------------------------------------------------------------------------
//Get integer value of field of given name, or default if not found
int Row::get_int(string fieldname, int def) const
{
  map<string,string>::const_iterator p=fields.find(fieldname);
  if (p!=fields.end()) return atoi(p->second.c_str());

  return def;
}

//------------------------------------------------------------------------
//Get 64-bit value of field of given name, or default if not found
uint64_t Row::get_int64(string fieldname, uint64_t def) const
{
  map<string,string>::const_iterator p=fields.find(fieldname);

  // Use sscanf instead of atoll which is signed
  if (p!=fields.end()) sscanf(p->second.c_str(), "%llu", &def);

  return def;
}

//------------------------------------------------------------------------
//Get boolean value of field of given name, or default if not found
bool Row::get_bool(string fieldname, bool def) const
{
  map<string,string>::const_iterator p=fields.find(fieldname);
  if (p!=fields.end())
  {
    char c=0;
    if (!p->second.empty()) c=p->second[0];
      
    switch(c)
    {
      case 'T': case 't':
      case 'Y': case 'y':
      case '1':
	return true;

      default:
	return false;
    }
  }

  return def;
}

//------------------------------------------------------------------------
// Get string with field names in order, separated by commas and spaces
string Row::get_fields() const
{
  string result;
  for(map<string, string>::const_iterator p = fields.begin(); 
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
  for(map<string, string>::const_iterator p = fields.begin(); 
      p!=fields.end(); ++p)
  {
    if (!result.empty()) result += ", ";
    result += "'" + escape(p->second) + "'";
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
  for(map<string, string>::const_iterator p = fields.begin(); 
      p!=fields.end(); ++p)
  {
    if (!result.empty()) result += ", ";
    result += p->first + " = '" + escape(p->second) + "'";
  }

  return result;
}

//------------------------------------------------------------------------
// Escape a string, doubling single quotes and backslashes
string Row::escape(const string& s)
{
  string s2 = Text::subst(s, "\\", "\\\\");
  return Text::subst(s2, "'", "''");
}

//------------------------------------------------------------------------
// Unescape a string, singling double quotes and backslashes
string Row::unescape(const string& s)
{
  string s2 = Text::subst(s, "\\\\", "\\");
  return Text::subst(s2, "''", "'");
}

}} // namespaces
