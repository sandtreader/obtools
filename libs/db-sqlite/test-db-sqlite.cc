//==========================================================================
// ObTools::DB: test-db-sqlite.cc
//
// Test harness for SQLite database wrapper
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-db-sqlite.h"
#include "ot-text.h"

using namespace std;
using namespace ObTools;

class DBSQLiteTest: public ::testing::Test
{
protected:
  static constexpr const auto dbfile = "/tmp/test.sqlite";

public:
  ~DBSQLiteTest()
  {
    unlink(dbfile);
  }
};

//--------------------------------------------------------------------------
// Tests
TEST_F(DBSQLiteTest, TestConnection)
{
  auto factory = DB::SQLite::ConnectionFactory(dbfile);
  auto conn = unique_ptr<DB::Connection>(factory.create());
  EXPECT_TRUE(conn.get());
}

TEST_F(DBSQLiteTest, TestCreateTable)
{
  auto factory = DB::SQLite::ConnectionFactory(dbfile);
  auto conn = unique_ptr<DB::Connection>(factory.create());
  EXPECT_TRUE(conn->exec("create table test (id int)"));
}

TEST_F(DBSQLiteTest, TestCreateTableAndRead)
{
  auto factory = DB::SQLite::ConnectionFactory(dbfile);
  auto conn = unique_ptr<DB::Connection>(factory.create());
  ASSERT_TRUE(conn->exec("create table test (id int)"));
  EXPECT_TRUE(conn->exec("insert into test (id) values (123)"));
  auto result = conn->query("select id from test");
  string value;
  ASSERT_TRUE(result.fetch(value));
  EXPECT_EQ("123", value);
}

TEST_F(DBSQLiteTest, TestCanReadWhilstWriteLocked)
{
  auto factory = DB::SQLite::ConnectionFactory(dbfile);
  auto conn = unique_ptr<DB::Connection>(factory.create());
  ASSERT_TRUE(conn->exec("create table test (id int)"));
  auto trans = DB::Transaction{*conn};
  for (auto i = 0; i < 1000; ++i)
    ASSERT_TRUE(conn->exec("insert into test (id) values (" +
                           Text::itos(i) + ")"));
  trans.commit();

  auto t1 = thread{[&factory]()
  {
    auto conn = unique_ptr<DB::Connection>(factory.create());
    auto trans = DB::Transaction{*conn};
    for (auto i = 1000; i < 1010; ++i)
      ASSERT_TRUE(conn->exec("insert into test (id) values (" +
                             Text::itos(i) + ")"));
    // Hold the write lock open a while
    this_thread::sleep_for(chrono::milliseconds{100});
    trans.commit();
  }};

  // Let thread start
  this_thread::sleep_for(chrono::milliseconds{10});

  {
    auto result = conn->query("select max(id) from test");
    string value;
    ASSERT_TRUE(result.fetch(value));
    EXPECT_EQ("999", value);
  }

  t1.join();

  string value;
  auto result2 = conn->query("select max(id) from test");
  ASSERT_TRUE(result2.fetch(value));
  EXPECT_EQ("1009", value);
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
