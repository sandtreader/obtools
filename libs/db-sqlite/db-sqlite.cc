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

namespace ObTools { namespace DB { namespace SQLite {

//==========================================================================
// SQLite result class

//--------------------------------------------------------------------------
// Constructor
ResultSet::ResultSet(sqlite3_stmt *_stmt):
  stmt{_stmt, sqlite3_finalize},
  num_fields{sqlite3_column_count(stmt.get())}
{
  for (auto i = 0; i < num_fields; ++i)
    field_names.push_back(sqlite3_column_name(stmt.get(), i));
}

//--------------------------------------------------------------------------
// Get number of rows in result set
int ResultSet::count()
{
  throw runtime_error("count() not supported for SQLite");
}

//--------------------------------------------------------------------------
// Get next row from result set
// Whether another was found - if so, clears and writes into row
bool ResultSet::fetch(Row& row)
{
  if (sqlite3_step(stmt.get()) != SQLITE_ROW)
      return false;

  row.clear();

  // Load all the fields the row
  for (auto i = 0; i < num_fields; ++i)
  {
    auto v = sqlite3_column_text(stmt.get(), i);
    if (v == nullptr)
      row.add_unescaped(field_names[i], "");
    else
      row.add_unescaped(field_names[i], reinterpret_cast<const char *>(v));
  }
  return true;
}

//------------------------------------------------------------------------
// Get first value of next row from result set
// Value is unescaped
// Whether another was found - if so, writes into value
bool ResultSet::fetch(string& value)
{
  if (sqlite3_step(stmt.get()) != SQLITE_ROW)
      return false;

  if (!num_fields)
    return false;

  auto v = sqlite3_column_text(stmt.get(), 0);
  if (v != nullptr)
    value = reinterpret_cast<const char *>(v);
  return true;
}

//==========================================================================
// SQLite connection class

//--------------------------------------------------------------------------
// Constructor
Connection::Connection(const string& file):
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
  valid = true;

  // Set up a busy handler
  sqlite3_busy_handler(c, [](void *, int attempts)
      {
        if (attempts > 8)
          return 0;
        this_thread::sleep_for(chrono::milliseconds{
                               static_cast<uint64_t>(pow(2, attempts))});
        return 1;
      }, nullptr);
}

//--------------------------------------------------------------------------
// Check whether connection is OK
bool Connection::ok()
{
  return valid;
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

//------------------------------------------------------------------------
// Execute a query and get result (e.g. SELECT)
// Returns result - check this for validity
Result Connection::query(const string& sql)
{
  OBTOOLS_LOG_IF_DEBUG({Log::Debug dlog; dlog << "DBquery: " << sql << endl;})

  char *err{nullptr};
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
    log << "SQLite query failed: " << err << endl;
  }
  return Result(new ResultSet{stmt});
}

}}} // namespaces
