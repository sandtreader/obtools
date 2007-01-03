//==========================================================================
// ObTools::DB: connection.cc
//
// Generic connection helper functions - call down into virtual subclass
// methods to get the work done
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-db.h"
#include <sstream>

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

//------------------------------------------------------------------------
//Do an INSERT and retrieve the last inserted serial ID
//Returns ID, or 0 if failed
int Connection::insert(const string& sql, 
		       const string& table, const string& id_field,
		       bool in_transaction)
{
  if (!in_transaction && !exec("START TRANSACTION")) return 0;
  if (!exec(sql)) return 0;

  // Assume autoincrementing IDs always increase, so max is the largest
  string sql2("SELECT max(");
  sql2 += id_field;
  sql2 += ") from ";
  sql2 += table;
  int id=query_int(sql2);

  if (!in_transaction) exec("COMMIT");

  return id;
}			 

//------------------------------------------------------------------------
// Do an INSERT and retrieve the last inserted serial ID, from row data
// Each field in the row is inserted by name
// Note: All fields are escaped on insertion
// Returns ID, or 0 if failed
int Connection::insert(Row& row, const string& table, const string& id_field,
		       bool in_transaction)
{
  ostringstream oss;
  oss << "INSERT INTO " << table;
  oss << " (" << row.get_fields() << ")";
  oss << " VALUES (" << row.get_escaped_values() << ")";
  return insert(oss.str(), table, id_field, in_transaction);
}

// !!! Do similar for:
//  SELECT fill in values in row based on where clause
//                ditto based on id value (int, string) and row
//  UPDATE ditto, but updating values from row
//  DELETE deleting from where clause, and id value (int, string)

}} // namespaces
