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

using namespace std;
using namespace ObTools;

class DBSQLiteTest: public ::testing::Test
{
protected:
  static constexpr const auto dbfile = "test.sqlite";

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

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
