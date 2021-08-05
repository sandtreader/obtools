//==========================================================================
// ObTools::FDB: test-database.cc
//
// Test harness for FDB C++ binding, database connection & transactions
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#include <gtest/gtest.h>
#include "ot-fdb.h"
#include "ot-log.h"

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Tests
TEST(FDBDatabaseTest, database_connects_and_disconnects)
{
  FDB::Database database;
  ASSERT_FALSE(!database);
}

TEST(FDBDatabaseTest, database_creates_a_transaction)
{
  FDB::Database database;
  ASSERT_FALSE(!database);
  FDB::Transaction transaction = database.create_transaction();
  ASSERT_FALSE(!transaction);
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_err = new Log::StreamChannel{&cerr};
    Log::logger.connect(chan_err);
  }

  FDB::Client client;
  client.start();

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
