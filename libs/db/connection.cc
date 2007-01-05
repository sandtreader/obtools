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
int Connection::insert(const string& table, Row& row, const string& id_field,
		       bool in_transaction)
{
  ostringstream oss;
  oss << "INSERT INTO " << table;
  oss << " (" << row.get_fields() << ")";
  oss << " VALUES (" << row.get_escaped_values() << ")";
  return insert(oss.str(), table, id_field, in_transaction);
}

//------------------------------------------------------------------------
// Do a SELECT for all fields in the given row in the given table 
// with the given WHERE clause
// If where is empty, doesn't add a WHERE at all
// Returns query result as query()
Result Connection::select(const string& table, const Row& row, 
			  const string& where)
{
  ostringstream oss;
  oss << "SELECT " << row.get_fields() << " FROM " << table;
  if (!where.empty()) oss << " WHERE " << where;
  return query(oss.str());
}

//------------------------------------------------------------------------
// Do a SELECT for all fields in the given row in the given table 
// matching the given integer ID
// Returns query result as query()
Result Connection::select_by_id(const string& table, const Row& row, 
				int id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = " << id;
  return select(table, row, oss.str());
}

//------------------------------------------------------------------------
// Do a SELECT for all fields in the given row in the given table 
// matching the given string ID
// ID value is escaped
// Returns query result as query()
Result Connection::select_by_id(const string& table, const Row& row, 
				const string& id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = '" << row.escape(id) << "'";
  return select(table, row, oss.str());
}

//------------------------------------------------------------------------
// Do a SELECT for all fields in the given row in the given table 
// with the given WHERE clause, and return the single (first) row as
// the values in the row
// If where is empty, doesn't add a WHERE at all
// Returns whether row fetched
bool Connection::select_row(const string& table, Row& row, 
			    const string& where)
{
  Result result = select(table, row, where);
  if (!result) return false;
  row.clear();
  return result.fetch(row);
}

//------------------------------------------------------------------------
// Do a SELECT for all fields in the given row in the given table 
// with the given integer ID, and return the single (first) row as
// the values in the row
// Returns whether row fetched
bool Connection::select_row_by_id(const string& table, Row& row,  
				  int id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = " << id;
  return select_row(table, row, oss.str());
}

//------------------------------------------------------------------------
// Do a SELECT for all fields in the given row in the given table 
// with the given string ID, and return the single (first) row as
// the values in the row
// ID value is escaped
// Returns whether row fetched
bool Connection::select_row_by_id(const string& table, Row& row,  
				  const string& id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = '" << row.escape(id) << "'";
  return select_row(table, row, oss.str());
}

//------------------------------------------------------------------------
// Do an UPDATE for all fields in the given row in the given table 
// with the given WHERE clause.  Values are escaped automatically
// If where is empty, doesn't add a WHERE at all
// Returns whether successful
bool Connection::update(const string& table, const Row& row, 
			const string& where)
{
  ostringstream oss;
  oss << "UPDATE " << table << " SET " << row.get_escaped_assignments();
  if (!where.empty()) oss << " WHERE " << where;
  return exec(oss.str());
}

//------------------------------------------------------------------------
// Do an UPDATE for all fields in the given row in the given table 
// matching the given integer ID
// Returns whether successful
bool Connection::update_id(const string& table, const Row& row, 
			   int id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = " << id;
  return update(table, row,  oss.str());
}

//------------------------------------------------------------------------
// Do an UPDATE for all fields in the given row in the given table 
// matching the given string ID
// ID value is escaped
// Returns whether successful
bool Connection::update_id(const string& table, const Row& row,  
			   const string& id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = '" << row.escape(id) << "'";
  return update(table, row,  oss.str());
}

//------------------------------------------------------------------------
// Do a DELETE in the given table with the given WHERE clause
// If where is empty, doesn't add a WHERE at all
// Returns whether successful
bool Connection::delete_all(const string& table, const string& where)
{
  ostringstream oss;
  oss << "DELETE FROM " << table;
  if (!where.empty()) oss << " WHERE " << where;
  return exec(oss.str());
}

//------------------------------------------------------------------------
// Do an DELETE in the given table matching the given integer ID
// Returns whether successful
bool Connection::delete_id(const string& table, 
			   int id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = " << id;
  return delete_all(table, oss.str());
}

//------------------------------------------------------------------------
// Do a DELETE in the given table matching the given string ID
// ID value is escaped
// Returns whether successful
bool Connection::delete_id(const string& table, 
			   const string& id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = '" << Row::escape(id) << "'";
  return delete_all(table, oss.str());
}

}} // namespaces
