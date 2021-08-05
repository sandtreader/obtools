//==========================================================================
// ObTools::FDB: database.cc
//
// FoundationDB API implementation: Database object
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#include "ot-fdb.h"
#include "ot-log.h"

namespace ObTools { namespace FDB {

// Constructor
Database::Database(const string& cluster_file_path)
{
  Log::Summary log;
  log << "Connecting to FDB database ";
  if (cluster_file_path.empty())
    log << "(default)\n";
  else
    log << "(cluster file " << cluster_file_path << ")\n";

  if (fdb_create_database(cluster_file_path.c_str(), &database))
  {
    Log::Error elog;
    elog << "Failed to connect to FDB database (" << cluster_file_path << ")\n";
    database = nullptr;
  }
}

// Create a transaction
Transaction Database::create_transaction()
{
  FDBTransaction *transaction;
  if (!database || fdb_database_create_transaction(database, &transaction))
  {
    Log::Error log;
    log << "Unable to create FDB database transaction\n";
    return Transaction(nullptr);
  }
  else return Transaction(transaction);
}

// Destructor
Database::~Database()
{
  if (database) fdb_database_destroy(database);
}

}} // namespaces
