//==========================================================================
// ObTools::DB: connection.cc
//
// Generic connection helper functions - call down into virtual subclass
// methods to get the work done
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#include "ot-db.h"

namespace ObTools { namespace DB {

//------------------------------------------------------------------------
//Execute a query and get first (only) row
//Returns whether successful - row is cleared and filled in if so
bool Connection::query(const string& sql, Row& row)
{
  Result result = query(sql);
  if (!result) return false;
  return result.fetch(row);
}

//------------------------------------------------------------------------
//Execute a query and get single (only) value from first (only) row
//Returns whether successful - value is filled in if so
bool Connection::query(const string& sql, string& value)
{
  Result result = query(sql);
  if (!result) return false;
  return result.fetch(value);
}

//------------------------------------------------------------------------
//Execute a query and get single (only) value from first (only) row
//Returns value or default if not found
string Connection::query_string(const string& sql, const string& def)
{
  string value;
  if (query(sql, value))
    return value;
  else
    return def;
}

//------------------------------------------------------------------------
//Execute a query and get single (only) integer value from first (only) row
//Returns value or default if not found
int Connection::query_int(const string& sql, int def)
{
  string value;
  if (query(sql, value))
    return atoi(value.c_str());
  else
    return def;
}

//------------------------------------------------------------------------
//Execute a query and get single (only) boolean value from first (only) row
//Returns value or default if not found
bool Connection::query_bool(const string& sql, bool def)
{
  string value;
  if (query(sql, value))
  {
    char c=0;
    if (!value.empty()) c=value[0];

    switch(c)
    {
      case 'T': case 't':
      case 'Y': case 'y':
	return true;

      default:
	return false;
    }
  }
  else return def;
}


}} // namespaces
