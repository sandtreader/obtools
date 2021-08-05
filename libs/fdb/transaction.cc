//==========================================================================
// ObTools::FDB: transaction.cc
//
// FoundationDB API implementation: Transaction object
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#include "ot-fdb.h"
#include "ot-log.h"

namespace ObTools { namespace FDB {

// Destructor
Transaction::~Transaction()
{
  if (transaction) fdb_transaction_destroy(transaction);
}

// Get a value
Future Transaction::get(const string& key, bool snapshot)
{
  if (!transaction) return Future(nullptr);
  return Future(
    fdb_transaction_get(transaction,
                        reinterpret_cast<const uint8_t *>(key.data()),
                        key.size(), snapshot));
}

// Set a value
bool Transaction::set(const string& key, const string& value)
{
  if (!transaction) return false;
  fdb_transaction_set(transaction,
                      reinterpret_cast<const uint8_t *>(key.data()),
                      key.size(),
                      reinterpret_cast<const uint8_t *>(value.data()),
                      value.size());
  return true;
}

// Clear a key
bool Transaction::clear(const string& key)
{
  if (!transaction) return false;
  fdb_transaction_clear(transaction,
                        reinterpret_cast<const uint8_t *>(key.data()),
                        key.size());
  return true;
}

// Commit - returns a Future with no value
Future Transaction::commit()
{
  if (!transaction) return Future(nullptr);
  return Future(fdb_transaction_commit(transaction));
}

}} // namespaces
