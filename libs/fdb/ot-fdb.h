//==========================================================================
// ObTools::FDB: ot-fdb.h
//
// Definition of C++ API for FoundationDB
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __OBTOOLS_FDB_H
#define __OBTOOLS_FDB_H

#include <string>
#include <ot-mt.h>

#define FDB_API_VERSION 630
#include "foundationdb/fdb_c.h"

namespace ObTools { namespace FDB {

// Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Singleton client - create this with the lifetime of the application
class Client
{
  bool running = false;

  // Background network thread class
  class NetworkThread: public MT::Thread
  {
    void run();
  };

  NetworkThread network_thread;

public:
  // Constructor - initialises API
  Client();

  // Start the network
  bool start();   // TODO: Take options

  // Stop the network
  void stop();

  // Destructor
  ~Client() { stop(); }
};

//==========================================================================
// Future object
class Future
{
  FDBFuture *future{nullptr};

public:
  // Constructor
  Future(FDBFuture* _future): future(_future) {}

  // Move constructor so we can pass these around
  Future(Future&& o): future(exchange(o.future, nullptr)) {}

  // Check validity
  bool operator!() { return !future; }

  // Check if it's ready (polling)
  bool poll() { return fdb_future_is_ready(future); }

  // Wait for it to be ready (blocking)
  bool wait() { return !fdb_future_block_until_ready(future); }

  // Get any error
  fdb_error_t get_error() { return future?fdb_future_get_error(future):-1; }

  // Get a string value
  string get_string(const string& def = "");

  // Cancel the future
  void cancel() { fdb_future_cancel(future); }

  // Destructor
  ~Future() { fdb_future_destroy(future); }
};

//==========================================================================
// Transaction object
class Transaction
{
  FDBTransaction *transaction{nullptr};

public:
  // Constructor
  Transaction(FDBTransaction *_transaction): transaction(_transaction) {}

  // Move constructor so we can pass these around
  Transaction(Transaction&& o): transaction(exchange(o.transaction, nullptr)) {}

  // Check validity
  bool operator!() { return !transaction; }

  // Get a value
  Future get(const string& key, bool snapshot = false);

  // Set a value
  bool set(const string& key, const string& value);

  // Clear a key
  bool clear(const string& key);

  // Commit - returns a Future with no value
  Future commit();

  // Destructor
  ~Transaction();
};

//==========================================================================
// Database object
class Database
{
  FDBDatabase *database{nullptr};

public:
  // Constructor
  Database(const string& cluster_file_path = "");

  // Move constructor so we can pass these around
  Database(Database&& o): database(exchange(o.database, nullptr)) {}

  // Check validity
  bool operator!() { return !database; }

  // Create a transaction
  Transaction create_transaction();

  // Destructor
  ~Database();
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_FDB_H
