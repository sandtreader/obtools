//==========================================================================
// ObTools::FDB: test-transaction.cc
//
// Test harness for FDB C++ binding, transactions
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
TEST(FDBTransactionTest, transaction_reads_back_its_own_write)
{
  FDB::Database database;
  ASSERT_FALSE(!database);
  FDB::Transaction transaction = database.create_transaction();
  ASSERT_FALSE(!transaction);

  ASSERT_TRUE(transaction.set("foo", "bar"));

  FDB::Future future = transaction.get("foo");
  ASSERT_FALSE(!future);
  ASSERT_TRUE(future.wait());
  ASSERT_EQ("bar", future.get_string());

  ASSERT_TRUE(transaction.clear("foo"));

  FDB::Future future2 = transaction.get("foo");
  ASSERT_FALSE(!future2);
  ASSERT_TRUE(future2.wait());
  ASSERT_EQ("", future2.get_string());
}

TEST(FDBTransactionTest, transaction_commits)
{
  FDB::Database database;
  ASSERT_FALSE(!database);
  FDB::Transaction transaction = database.create_transaction();
  ASSERT_FALSE(!transaction);

  ASSERT_TRUE(transaction.set("foo", "bar"));
  auto future = transaction.commit();
  ASSERT_TRUE(future.wait());
  ASSERT_EQ(0, future.get_error());
}

TEST(FDBTransactionTest, transaction_persists) // note relies on previous
{
  FDB::Database database;
  ASSERT_FALSE(!database);
  FDB::Transaction transaction = database.create_transaction();
  ASSERT_FALSE(!transaction);

  FDB::Future future = transaction.get("foo");
  ASSERT_FALSE(!future);
  ASSERT_TRUE(future.wait());
  ASSERT_EQ("bar", future.get_string());

  ASSERT_TRUE(transaction.clear("foo"));

  auto future2 = transaction.commit();
  ASSERT_TRUE(future2.wait());
  ASSERT_EQ(0, future2.get_error());
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
