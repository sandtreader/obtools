//==========================================================================
// ObTools::DB: transaction.cc
//
// Transaction holder
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-db.h"
#ifdef DEBUG
#include "ot-log.h"
#include <execinfo.h>
#endif

namespace ObTools { namespace DB {

//--------------------------------------------------------------------------
// Constructor from plain connection
Transaction::Transaction(Connection& _conn): conn(&_conn), committed(false)
{
  if (!conn->exec("BEGIN")) committed = true;  // Make commit fail
}

//--------------------------------------------------------------------------
// Constructor from AutoConnection
Transaction::Transaction(AutoConnection& _autoconn):
  conn(_autoconn.conn), committed(false)
{
  if (!conn || !conn->exec("BEGIN"))
    committed = true;  // Make commit fail
}

//--------------------------------------------------------------------------
// Commit - returns whether commit command ran OK
bool Transaction::commit()
{
  if (!conn || committed) return false;
#ifdef DEBUG
  const auto commit_start = chrono::high_resolution_clock::now();
#endif
  committed = conn->exec("COMMIT");
#ifdef DEBUG
  const auto spent = chrono::high_resolution_clock::now() - start;
  if (spent > chrono::milliseconds{1000})
  {
    const auto commit_spent = chrono::high_resolution_clock::now()
                              - commit_start;
    Log::Error log;
    constexpr auto buff_size = 2;
    void *buffer[buff_size];
    auto n = backtrace(buffer, buff_size);
    auto **lines = backtrace_symbols(buffer, n);
    auto line = string{};
    if (lines)
    {
      if (n > 1)
        line = lines[1];
      free(lines);
    }
    log << "Slow transaction ("
        << chrono::duration_cast<chrono::milliseconds>(spent).count()
        << "ms / commit "
        << chrono::duration_cast<chrono::milliseconds>(commit_spent).count()
        << "ms): " << line << endl;
  }
#endif
  return committed;
}

//--------------------------------------------------------------------------
// Destructor
Transaction::~Transaction()
{
  if (conn && !committed) conn->exec("ROLLBACK");
}



}} // namespaces
