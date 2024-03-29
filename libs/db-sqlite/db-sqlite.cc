//==========================================================================
// ObTools::DB: db-sqlite.cc
//
// SQLite Database wrapper implementation
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-db-sqlite.h"
#include "ot-log.h"
#include <sqlite3.h>
#include <cmath>
#include <sstream>

namespace ObTools { namespace DB { namespace SQLite {

//==========================================================================
// SQLite prepare statement class

//--------------------------------------------------------------------------
// Constructor
PreparedStatement::PreparedStatement(sqlite3_stmt *_stmt):
  stmt{_stmt, sqlite3_finalize},
  num_fields{sqlite3_column_count(stmt.get())}
{
  for (auto i = 0; i < num_fields; ++i)
    field_names.push_back(sqlite3_column_name(stmt.get(), i));
}

//--------------------------------------------------------------------------
// Bind a parameter (integer)
bool PreparedStatement::bind(int index, int64_t value)
{
  return sqlite3_bind_int64(stmt.get(), index, value) == SQLITE_OK;
}

//--------------------------------------------------------------------------
// Bind a parameter (real)
bool PreparedStatement::bind(int index, double value)
{
  return sqlite3_bind_double(stmt.get(), index, value) == SQLITE_OK;
}

//--------------------------------------------------------------------------
// Bind a parameter (text)
bool PreparedStatement::bind(int index, const string& value)
{
  return sqlite3_bind_text(stmt.get(), index, value.c_str(),
                           value.size(), SQLITE_TRANSIENT) == SQLITE_OK;
}

//--------------------------------------------------------------------------
// Bind a parameter (null)
bool PreparedStatement::bind(int index)
{
  return sqlite3_bind_null(stmt.get(), index) == SQLITE_OK;
}

//--------------------------------------------------------------------------
// Reset statement
void PreparedStatement::reset()
{
  sqlite3_reset(stmt.get());
}

//--------------------------------------------------------------------------
// Execute statement
bool PreparedStatement::execute()
{
  const auto result = sqlite3_step(stmt.get());
  if (result != SQLITE_DONE)
  {
    Log::Error log;
    log << "SQLite statement failed to execute: "
        << sqlite3_errstr(result) << endl;
  }
  return result == SQLITE_DONE;
}

//--------------------------------------------------------------------------
// Get number of rows in result set
int PreparedStatement::count()
{
  throw runtime_error("count() not supported for SQLite");
}

//--------------------------------------------------------------------------
// Get next row from result set
// Whether another was found - if so, clears and writes into row
bool PreparedStatement::fetch(Row& row)
{
  if (sqlite3_step(stmt.get()) != SQLITE_ROW)
    return false;

  row.clear();

  // Load all the fields the row
  for (auto i = 0; i < num_fields; ++i)
  {
    auto v = sqlite3_column_text(stmt.get(), i);
    if (v == nullptr)
      row.add(field_names[i], "");
    else
      row.add(field_names[i], reinterpret_cast<const char *>(v));
  }
  return true;
}

//--------------------------------------------------------------------------
// Get first value of next row from result set
// Value is unescaped
// Whether another was found - if so, writes into value
bool PreparedStatement::fetch(string& value)
{
  if (!num_fields)
    return false;

  if (sqlite3_step(stmt.get()) != SQLITE_ROW)
    return false;

  auto v = sqlite3_column_text(stmt.get(), 0);
  if (v != nullptr)
    value = reinterpret_cast<const char *>(v);
  return true;
}

//--------------------------------------------------------------------------
// Move to next row
bool PreparedStatement::next()
{
  return sqlite3_step(stmt.get()) == SQLITE_ROW;
}

//--------------------------------------------------------------------------
// Fetch field as string
string PreparedStatement::get_string(int col)
{
  auto v = sqlite3_column_text(stmt.get(), col);
  if (v == nullptr)
    return {};
  return reinterpret_cast<const char *>(v);
}

//--------------------------------------------------------------------------
// Fetch field as int
uint64_t PreparedStatement::get_int(int col)
{
  return sqlite3_column_int64(stmt.get(), col);
}

//--------------------------------------------------------------------------
// Fetch field as double
double PreparedStatement::get_real(int col)
{
  return sqlite3_column_double(stmt.get(), col);
}

//--------------------------------------------------------------------------
// Fetch field as Time::Stamp
Time::Stamp PreparedStatement::get_time(int col)
{
  auto v = sqlite3_column_text(stmt.get(), col);
  if (v == nullptr)
    return {};
  return Time::Stamp{string{reinterpret_cast<const char *>(v)}};
}

//==========================================================================
// SQLite connection class

//--------------------------------------------------------------------------
// Constructor
Connection::Connection(const string& file, const Time::Duration& timeout):
  conn{nullptr, sqlite3_close}
{
  sqlite3 *c{nullptr};
  Log::Streams log;

  auto e = sqlite3_open(file.c_str(), &c);
  conn.reset(c);
  if (e != SQLITE_OK)
  {
    log.error << "DB: Can't open SQLite " << file << ": "
              << sqlite3_errmsg(conn.get()) << endl;
    return;
  }

  // OK, we have a connection
  log.detail << "SQLite connection opened to " << file << endl;

  // Set up a busy timeout
  sqlite3_busy_timeout(c, timeout.milliseconds());

  // Turn on foreign key checks (really?!)
  exec("PRAGMA foreign_keys = ON");
}

//--------------------------------------------------------------------------
// Check whether connection is OK
Connection::operator bool()
{
  return conn.get();
}

//--------------------------------------------------------------------------
// Execute a command, not expecting any result (e.g. INSERT, UPDATE, DELETE)
// Returns whether successful
bool Connection::exec(const string& sql)
{
  OBTOOLS_LOG_IF_DEBUG({Log::Debug dlog; dlog << "DBexec: " << sql << endl;})

  char *err{nullptr};
  auto e = sqlite3_exec(conn.get(), sql.c_str(), nullptr, nullptr, &err);
  if (e != SQLITE_OK)
  {
    Log::Error log;
    log << "SQLite exec failed: " << err << endl;
    return false;
  }

  OBTOOLS_LOG_IF_DEBUG({Log::Debug dlog; dlog << "DBexec OK" << endl;})
  return true;
}

//--------------------------------------------------------------------------
// Execute a query and get result (e.g. SELECT)
// Returns result - check this for validity
Result Connection::query(const string& sql)
{
  OBTOOLS_LOG_IF_DEBUG({Log::Debug dlog; dlog << "DBquery: " << sql << endl;})

  sqlite3_stmt *stmt{0};
  auto e = sqlite3_prepare_v2(conn.get(), sql.c_str(), sql.size(), &stmt,
                              nullptr);
  if (e == SQLITE_OK)
  {
    OBTOOLS_LOG_IF_DEBUG({Log::Debug dlog; dlog << "DBquery OK" << endl;})
  }
  else
  {
    Log::Error log;
    log << "SQLite query failed: " << sqlite3_errmsg(conn.get()) << endl;
  }
  return Result(new ResultSet{stmt});
}

//--------------------------------------------------------------------------
// Prepare a statement
// Returns result - check this for validity
Statement Connection::prepare(const string& sql)
{
  OBTOOLS_LOG_IF_DEBUG({Log::Debug dlog; dlog << "DBprepare: " << sql << endl;})

  sqlite3_stmt *stmt{0};
  auto e = sqlite3_prepare_v2(conn.get(), sql.c_str(), sql.size(), &stmt,
                              nullptr);
  if (e == SQLITE_OK)
  {
    OBTOOLS_LOG_IF_DEBUG({Log::Debug dlog; dlog << "DBprepare OK" << endl;})
  }
  else
  {
    Log::Error log;
    log << "SQLite prepare failed: " << sqlite3_errmsg(conn.get()) << endl;
  }
  return Statement(new PreparedStatement{stmt});
}

//--------------------------------------------------------------------------
// Gets the last insert id
uint64_t Connection::get_last_insert_id()
{
  return sqlite3_last_insert_rowid(conn.get());
}

//--------------------------------------------------------------------------
// Do an INSERT or UPDATE if it already exists (violates unique key)
// Uses INSERT ... ON CONFLICT DO UPDATE
// Each field in the row is inserted by name
// update_row gives the list of fields from row which are updated (not
// part of the unique key)
// Note: All fields are escaped on insertion
// Returns whether successful
bool Connection::insert_or_update(const string& table, Row& row,
                                  Row& update_row)
{
  ostringstream oss;
  oss << "INSERT INTO " << table;
  oss << " (" << row.get_fields() << ")";
  oss << " VALUES (" << row.get_escaped_values() << ")";
  oss << " ON CONFLICT (" << row.get_fields_not_in(update_row)
      << ") DO UPDATE SET "
      << row.get_escaped_assignments_limited_by(update_row);
  return exec(oss.str());
}

}}} // namespaces
