//==========================================================================
// ObTools::DB: connection.cc
//
// Generic connection helper functions - call down into virtual subclass
// methods to get the work done
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-db.h"
#include "ot-text.h"
#include "ot-log.h"
#include <stdlib.h>
#include <sstream>

namespace ObTools { namespace DB {

//--------------------------------------------------------------------------
// Create a held prepared statement
bool Connection::prepare_statement(const string& id, const string& sql)
{
  auto stmt = prepare(sql);
  if (!stmt)
    return false;
  prepared_statements[id] = move(stmt);
  return true;
}

//--------------------------------------------------------------------------
// Get a held prepared statement for use
// Note: user is responsible for ensuring thread safety
AutoStatement Connection::get_statement(const string& id)
{
  auto it = prepared_statements.find(id);
  if (it == prepared_statements.end())
  {
    Log::Error log;
    log << "Failed to get prepared statement id " << id << endl;
    return nullptr;
  }
  return &it->second;
}

//--------------------------------------------------------------------------
// Execute a query and get first (only) row
// Returns whether successful - row is cleared and filled in if so
bool Connection::query(const string& sql, Row& row)
{
  Result result = query(sql);
  if (!result) return false;
  return result.fetch(row);
}

//--------------------------------------------------------------------------
// Execute a query and get single (only) value from first (only) row
// Returns whether successful - value is filled in if so
bool Connection::query(const string& sql, string& value)
{
  Result result = query(sql);
  if (!result) return false;
  return result.fetch(value);
}

//--------------------------------------------------------------------------
// Execute a query and get single (only) value from first (only) row
// Returns value or default if not found
string Connection::query_string(const string& sql, const string& def)
{
  string value;
  if (query(sql, value))
    return value;
  else
    return def;
}

//--------------------------------------------------------------------------
// Execute a query and get single (only) integer value from first (only) row
// Returns value or default if not found
int Connection::query_int(const string& sql, int def)
{
  string value;
  if (query(sql, value))
    return atoi(value.c_str());
  else
    return def;
}

//--------------------------------------------------------------------------
// Execute a query and get single (only) 64-bit integer value from first (only)
// row
// Returns value or default if not found
uint64_t Connection::query_int64(const string& sql, uint64_t def)
{
  string value;
  if (query(sql, value))
    return Text::stoi64(value.c_str());
  else
    return def;
}

//--------------------------------------------------------------------------
// Execute a query and get single (only) boolean value from first (only) row
// Returns value or default if not found
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

//--------------------------------------------------------------------------
// Do an INSERT and retrieve the last inserted serial ID
// Fetches max(<id_field>) inside a transaction, unless id_field is ""
// If id_field is "", does a normal insert and returns 1
// Returns ID, or 0 if failed
int Connection::insert(const string& sql,
                       const string& table, const string& id_field,
                       bool in_transaction)
{
  // Allow for not interested in returned ID - simple version
  if (id_field.empty()) return exec(sql)?1:0;

  // Want ID back in transaction
  if (!in_transaction && !exec("START TRANSACTION")) return 0;
  if (!exec(sql))
  {
    if (!in_transaction) exec("ROLLBACK");  // Try to roll back
    return 0;
  }

  // Assume autoincrementing IDs always increase, so max is the largest
  string sql2("SELECT max(");
  sql2 += id_field;
  sql2 += ") from ";
  sql2 += table;
  int id=query_int(sql2);

  if (!in_transaction) exec("COMMIT");

  return id;
}

//--------------------------------------------------------------------------
// Ditto for 64-bit
uint64_t Connection::insert64(const string& sql,
                              const string& table, const string& id_field,
                              bool in_transaction)
{
  // Allow for not interested in returned ID - simple version
  if (id_field.empty()) return exec(sql)?1:0;

  // Want ID back in transaction
  if (!in_transaction && !exec("START TRANSACTION")) return 0;
  if (!exec(sql))
  {
    if (!in_transaction) exec("ROLLBACK");  // Try to roll back
    return 0;
  }

  // Assume autoincrementing IDs always increase, so max is the largest
  string sql2("SELECT max(");
  sql2 += id_field;
  sql2 += ") from ";
  sql2 += table;
  uint64_t id=query_int64(sql2);

  if (!in_transaction) exec("COMMIT");

  return id;
}

//--------------------------------------------------------------------------
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

// Ditto with 64-bit
uint64_t Connection::insert64(const string& table, Row& row,
                              const string& id_field,
                              bool in_transaction)
{
  ostringstream oss;
  oss << "INSERT INTO " << table;
  oss << " (" << row.get_fields() << ")";
  oss << " VALUES (" << row.get_escaped_values() << ")";
  return insert64(oss.str(), table, id_field, in_transaction);
}

//--------------------------------------------------------------------------
// INSERT into a join table with two foreign ID fields
// Returns whether successful
bool Connection::insert_join(const string& table,
                             const string& field1, int id1,
                             const string& field2, int id2)
{
  ostringstream oss;
  oss << "INSERT INTO " << table;
  oss << " (" << field1 << ", " << field2 << ")";
  oss << " VALUES (" << id1 << ", " << id2 << ")";
  return exec(oss.str());
}

// Ditto with 64-bit
bool Connection::insert_join64(const string& table,
                               const string& field1, uint64_t id1,
                               const string& field2, uint64_t id2)
{
  ostringstream oss;
  oss << "INSERT INTO " << table;
  oss << " (" << field1 << ", " << field2 << ")";
  oss << " VALUES (" << id1 << ", " << id2 << ")";
  return exec(oss.str());
}

//--------------------------------------------------------------------------
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

//--------------------------------------------------------------------------
// Do a SELECT for all fields in the given row in the given table
// matching the list of values in where_row
// Returns query result as query()
Result Connection::select(const string& table, const Row& row,
                          const Row& where_row)
{
  return select(table, row, where_row.get_where_clause());
}

//--------------------------------------------------------------------------
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

// Ditto, 64-bit
Result Connection::select_by_id64(const string& table, const Row& row,
                                  uint64_t id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = " << id;
  return select(table, row, oss.str());
}

//--------------------------------------------------------------------------
// Do a SELECT for all fields in the given row in the given table
// matching the given string ID
// ID value is escaped
// Returns query result as query()
Result Connection::select_by_id(const string& table, const Row& row,
                                const string& id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = " << FieldValue::quote(id);
  return select(table, row, oss.str());
}

//--------------------------------------------------------------------------
// Do a SELECT for all fields in the given row in the given table
// with the given WHERE clause, and return the single (first) row as
// the values in the row
// If where is empty, doesn't add a WHERE at all
// Returns whether row fetched
bool Connection::select_row(const string& table, Row& row,
                            const string& where)
{
  string limit_where = where+" LIMIT 1";
  Result result = select(table, row, limit_where);
  if (!result) return false;
  row.clear();
  return result.fetch(row);
}

//--------------------------------------------------------------------------
// Do a SELECT for all fields in the given row in the given table
// with a WHERE clause constructed from where_row, and return the single
// (first) row as the values in the row
// Returns whether row fetched
bool Connection::select_row(const string& table, Row& row,
                            const Row& where_row)
{
  return select_row(table, row, where_row.get_where_clause());
}

//--------------------------------------------------------------------------
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

// Ditto, 64-bit
bool Connection::select_row_by_id64(const string& table, Row& row,
                                    uint64_t id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = " << id;
  return select_row(table, row, oss.str());
}

//--------------------------------------------------------------------------
// Do a SELECT for all fields in the given row in the given table
// with the given string ID, and return the single (first) row as
// the values in the row
// ID value is escaped
// Returns whether row fetched
bool Connection::select_row_by_id(const string& table, Row& row,
                                  const string& id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = " << FieldValue::quote(id);
  return select_row(table, row, oss.str());
}

//--------------------------------------------------------------------------
// Do a SELECT for a single field in the given table
// with the given WHERE clause, and return the (unescaped) value
// If where is empty, doesn't add a WHERE at all
// Returns value or empty string if not found
string Connection::select_value(const string& table, const string& field,
                                const string& where)
{
  ostringstream oss;
  oss << "SELECT " << field << " FROM " << table;
  if (!where.empty()) oss << " WHERE " << where;
  oss << " LIMIT 1";
  return query_string(oss.str());
}

//--------------------------------------------------------------------------
// Do a SELECT for a single field in the given table
// with a WHERE clause constructed from where_row, and return the
// (unescaped) value
// Returns value or empty string if not found
string Connection::select_value(const string& table, const string& field,
                                const Row& where_row)
{
  return select_value(table, field, where_row.get_where_clause());
}

//--------------------------------------------------------------------------
// Do a SELECT for a single field in the given table
// with the given integer ID, and return the (unescaped) value
// Returns value or empty string if not found
string Connection::select_value_by_id(const string& table,
                                      const string& field,
                                      int id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = " << id;
  return select_value(table, field, oss.str());
}

// Ditto, 64-bit
string Connection::select_value_by_id64(const string& table,
                                        const string& field,
                                        uint64_t id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = " << id;
  return select_value(table, field, oss.str());
}

//--------------------------------------------------------------------------
// Do a SELECT for a single field in the given table
// with the given integer ID, and return the (unescaped) value
// ID value is escaped
// Returns value or empty string if not found
string Connection::select_value_by_id(const string& table,
                                      const string& field,
                                      const string& id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = " << FieldValue::quote(id);
  return select_value(table, field, oss.str());
}

//--------------------------------------------------------------------------
// Count rows in the given table with the given WHERE clause
// If where is empty, doesn't add a WHERE at all
int Connection::count(const string& table, const string& where)
{
  ostringstream oss;
  oss << "SELECT COUNT(*) FROM " << table;
  if (!where.empty()) oss << " WHERE " << where;
  return query_int(oss.str());
}

//--------------------------------------------------------------------------
// Count rows in the given table matching the list of values in where_row
int Connection::count(const string& table, const Row& where_row)
{
  return count(table, where_row.get_where_clause());
}

//--------------------------------------------------------------------------
// Check if a row exists with the given integer ID
// Returns whether the row exists
bool Connection::exists_id(const string& table,
                           int id, const string& id_field)
{
  return !select_value_by_id(table, id_field, id, id_field).empty();
}

// Ditto, 64-bit
bool Connection::exists_id64(const string& table,
                             uint64_t id, const string& id_field)
{
  return !select_value_by_id64(table, id_field, id, id_field).empty();
}

//--------------------------------------------------------------------------
// Check if a row exists with the given string ID
// ID value is escaped
// Returns whether the row exists
bool Connection::exists_id(const string& table,
                           const string& id, const string& id_field)
{
  return !select_value_by_id(table, id_field, id, id_field).empty();
}

//--------------------------------------------------------------------------
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

//--------------------------------------------------------------------------
// Do an UPDATE for all fields in the given row in the given table
// with a WHERE clause created from where_row.
// Values are escaped automatically
// Returns whether successful
bool Connection::update(const string& table, const Row& row,
                        const Row& where_row)
{
  return update(table, row, where_row.get_where_clause());
}

//--------------------------------------------------------------------------
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

// Ditto, 64-bit
bool Connection::update_id64(const string& table, const Row& row,
                             uint64_t id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = " << id;
  return update(table, row,  oss.str());
}

//--------------------------------------------------------------------------
// Do an UPDATE for all fields in the given row in the given table
// matching the given string ID
// ID value is escaped
// Returns whether successful
bool Connection::update_id(const string& table, const Row& row,
                           const string& id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = " << FieldValue::quote(id);
  return update(table, row,  oss.str());
}

//--------------------------------------------------------------------------
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

//--------------------------------------------------------------------------
// Do a DELETE in the given table with a WHERE clause created from where_row
// Returns whether successful
bool Connection::delete_all(const string& table, const Row& where_row)
{
  return delete_all(table, where_row.get_where_clause());
}

//--------------------------------------------------------------------------
// Do an DELETE in the given table matching the given integer ID
// Returns whether successful
bool Connection::delete_id(const string& table,
                           int id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = " << id;
  return delete_all(table, oss.str());
}

// Ditto, 64-bit
bool Connection::delete_id64(const string& table,
                             uint64_t id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = " << id;
  return delete_all(table, oss.str());
}

//--------------------------------------------------------------------------
// Do a DELETE in the given table matching the given string ID
// ID value is escaped
// Returns whether successful
bool Connection::delete_id(const string& table,
                           const string& id, const string& id_field)
{
  ostringstream oss;
  oss << id_field << " = " << FieldValue::quote(id);
  return delete_all(table, oss.str());
}

//--------------------------------------------------------------------------
// DELETE from a join table with two foreign ID fields
// Returns whether successful
bool Connection::delete_join(const string& table,
                             const string& field1, int id1,
                             const string& field2, int id2)
{
  ostringstream oss;
  oss << field1 << " = " << id1 << " AND " << field2 << " = " << id2;
  return delete_all(table, oss.str());
}

// Ditto, 64-bit
bool Connection::delete_join64(const string& table,
                               const string& field1, uint64_t id1,
                               const string& field2, uint64_t id2)
{
  ostringstream oss;
  oss << field1 << " = " << id1 << " AND " << field2 << " = " << id2;
  return delete_all(table, oss.str());
}

//==========================================================================
// Connection Factory

//--------------------------------------------------------------------------
// create a new connection
Connection *ConnectionFactory::create()
{
  auto conn = create_connection();
  if (conn)
  {
    for (const auto& stmt: prepared_statements)
    {
      if (!conn->prepare_statement(stmt.first, stmt.second))
      {
        Log::Error log;
        log << "Failed to prepare statement id " << stmt.first << endl;
      }
    }
  }
  return conn;
}

}} // namespaces
