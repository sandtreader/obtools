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
#ifndef __WIN32__
#include <execinfo.h>
#endif
#endif

namespace ObTools { namespace DB {

//--------------------------------------------------------------------------
// Constructor from plain connection
Transaction::Transaction(Connection& _conn, bool immediate):
  conn(&_conn), committed(false)
{
  if (immediate)
    committed = !conn->transaction_begin_immediate();
  else
    committed = !conn->transaction_begin();
#ifdef DEBUG
  begun_at = chrono::high_resolution_clock::now() - start;
#endif
}

//--------------------------------------------------------------------------
// Constructor from AutoConnection
Transaction::Transaction(AutoConnection& _autoconn, bool immediate):
  conn(_autoconn.conn), committed(false)
{
  if (!conn)
  {
    committed = true;
    return;
  }
  if (immediate)
    committed = !conn->transaction_begin_immediate();
  else
    committed = !conn->transaction_begin();
#ifdef DEBUG
  begun_at = chrono::high_resolution_clock::now() - start;
#endif
}

//--------------------------------------------------------------------------
// Commit - returns whether commit command ran OK
bool Transaction::commit()
{
  if (!conn || committed) return false;
#ifdef DEBUG
  const auto commit_at = chrono::high_resolution_clock::now() - start;
#endif
  committed = conn->transaction_commit();
#ifdef DEBUG
  const auto spent = chrono::high_resolution_clock::now() - start;
  if (spent > chrono::milliseconds{1000})
  {
    Log::Error log;
#ifdef __WIN32__
    log << "Slow transaction ("
        << chrono::duration_cast<chrono::milliseconds>(spent).count()
        << "ms "
        << "/ begun at "
        << chrono::duration_cast<chrono::milliseconds>(begun_at).count()
        << "ms "
        << "/ commit at "
        << chrono::duration_cast<chrono::milliseconds>(commit_at).count()
        << "ms)" << endl;
#else
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
        << "ms "
        << "/ begun at "
        << chrono::duration_cast<chrono::milliseconds>(begun_at).count()
        << "ms "
        << "/ commit at "
        << chrono::duration_cast<chrono::milliseconds>(commit_at).count()
        << "ms): " << line << endl;
#endif
  }
#endif
  return committed;
}

//--------------------------------------------------------------------------
// Destructor
Transaction::~Transaction()
{
  if (conn && !committed) conn->transaction_rollback();
}



}} // namespaces
