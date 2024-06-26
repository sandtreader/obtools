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
#include "ot-file.h"

using namespace std;
using namespace ObTools;

namespace {
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

const auto timeout = Time::Duration{"1 minute"};
}

//--------------------------------------------------------------------------
// Tests
TEST_F(DBSQLiteTest, TestConnection)
{
  auto factory = DB::SQLite::ConnectionFactory(dbfile, timeout);
  auto conn = unique_ptr<DB::Connection>(factory.create());
  EXPECT_TRUE(conn.get());
}

TEST_F(DBSQLiteTest, TestCreateTable)
{
  auto factory = DB::SQLite::ConnectionFactory(dbfile, timeout);
  auto conn = unique_ptr<DB::Connection>(factory.create());
  EXPECT_TRUE(conn->exec("create table test (id int)"));
}

TEST_F(DBSQLiteTest, TestCreateTableAndRead)
{
  auto factory = DB::SQLite::ConnectionFactory(dbfile, timeout);
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
  auto factory = DB::SQLite::ConnectionFactory(dbfile, timeout);
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

TEST_F(DBSQLiteTest, TestPreparedStatement)
{
  auto factory = DB::SQLite::ConnectionFactory(dbfile, timeout);
  auto conn = unique_ptr<DB::Connection>(factory.create());
  ASSERT_TRUE(conn->exec("create table test (id int, name text)"));
  ASSERT_TRUE(conn->exec("insert into test (id, name) values (123, 'foo')"));

  auto statement = conn->prepare("select name from test where id = ?");
  ASSERT_TRUE(!!statement);
  ASSERT_TRUE(statement.bind(1, uint64_t{123}));
  auto value = string{};
  ASSERT_TRUE(statement.fetch(value));
  EXPECT_EQ("foo", value);
}

TEST_F(DBSQLiteTest, TestPreparedStatementReuse)
{
  auto factory = DB::SQLite::ConnectionFactory(dbfile, timeout);
  auto conn = unique_ptr<DB::Connection>(factory.create());
  ASSERT_TRUE(conn->exec("create table test (id int, name text)"));
  ASSERT_TRUE(conn->exec("insert into test (id, name) values (123, 'foo')"));
  ASSERT_TRUE(conn->exec("insert into test (id, name) values (456, 'bar')"));

  auto statement = conn->prepare("select name from test where id = ?");
  ASSERT_TRUE(!!statement);
  ASSERT_TRUE(statement.bind(1, uint64_t{123}));
  auto value = string{};
  ASSERT_TRUE(statement.fetch(value));
  ASSERT_EQ("foo", value);
  statement.reset();
  ASSERT_TRUE(statement.bind(1, uint64_t{456}));
  ASSERT_TRUE(statement.fetch(value));
  EXPECT_EQ("bar", value);
}

TEST_F(DBSQLiteTest, TestHeldPreparedStatementDoesNotHoldLock)
{
  auto factory = DB::SQLite::ConnectionFactory(dbfile, timeout);
  auto conn = unique_ptr<DB::Connection>(factory.create());
  ASSERT_TRUE(conn->exec("create table test (id int, name text)"));

  auto trans = DB::Transaction{*conn};
  auto statement = conn->prepare("insert into test(id, name) values (?, ?)");
  ASSERT_TRUE(!!statement);
  ASSERT_TRUE(statement.bind(1, uint64_t{123}));
  ASSERT_TRUE(statement.bind(1, "foo"));
  ASSERT_TRUE(statement.execute());
  trans.commit();
  ASSERT_FALSE(File::Path(string{dbfile} + "-journal").exists());
}

TEST_F(DBSQLiteTest, TestLastInsertId)
{
  auto factory = DB::SQLite::ConnectionFactory(dbfile, timeout);
  auto conn = unique_ptr<DB::Connection>(factory.create());
  ASSERT_TRUE(conn->exec("create table test (id int primary key, a text)"));

  auto trans = DB::Transaction{*conn};
  auto statement = conn->prepare("insert into test(a) values ('a')");
  const auto expected = 42;
  for (auto i = 0; i < expected; ++i)
    ASSERT_TRUE(statement.execute());
  EXPECT_EQ(expected, conn->get_last_insert_id());
  trans.commit();
}

TEST_F(DBSQLiteTest, TestMultithreadingWithConnectionPerThread)
{
  auto factory = DB::SQLite::ConnectionFactory(dbfile, timeout);
  auto conn = unique_ptr<DB::Connection>(factory.create());
  ASSERT_TRUE(conn->exec("create table test (id int)"));

  vector<thread> threads;

  for(auto i=0; i<100; i++)
  {
    threads.push_back(thread{[&factory, i]()
    {
      auto conn = unique_ptr<DB::Connection>(factory.create());

      for(auto j=0; j<10; j++)
      {
        // Half reading, half writing
        if (i%2)
        {
          auto trans = DB::Transaction{*conn};
          ASSERT_TRUE(conn->exec("insert into test (id) values (" +
                                 Text::itos(10*i+j) + ")"));
          trans.commit();
        }
        else
        {
          auto result = conn->query("select max(id) from test");
          string value;
          ASSERT_TRUE(result.fetch(value));
        }
      }
    }});
  }

  for(auto& t: threads)
    t.join();

  string value;
  auto result2 = conn->query("select max(id) from test");
  ASSERT_TRUE(result2.fetch(value));
  EXPECT_EQ("999", value);
}

TEST_F(DBSQLiteTest, TestMultithreadingWithSharedConnection)
{
  auto factory = DB::SQLite::ConnectionFactory(dbfile, timeout);
  auto conn = unique_ptr<DB::Connection>(factory.create());
  ASSERT_TRUE(conn->exec("create table test (id int)"));

  vector<thread> threads;

  for(auto i=0; i<100; i++)
  {
    threads.push_back(thread{[&conn, i]()
    {
      for(auto j=0; j<10; j++)
      {
        // Half reading, half writing
        if (i%2)
        {
          auto trans = DB::Transaction{*conn};
          ASSERT_TRUE(conn->exec("insert into test (id) values (" +
                                 Text::itos(10*i+j) + ")"));
          trans.commit();
        }
        else
        {
          auto result = conn->query("select max(id) from test");
          string value;
          ASSERT_TRUE(result.fetch(value));
        }
      }
    }});
  }

  for(auto& t: threads)
    t.join();

  string value;
  auto result2 = conn->query("select max(id) from test");
  ASSERT_TRUE(result2.fetch(value));
  EXPECT_EQ("999", value);
}

TEST_F(DBSQLiteTest, TestMultithreadingWithPooledConnectionPerThread)
{
  auto factory = DB::SQLite::ConnectionFactory(dbfile, timeout);
  DB::ConnectionPool pool(factory, 0, 25, Time::Duration("10s"));
  DB::AutoConnection conn(pool);
  ASSERT_TRUE(conn.exec("create table test (id int)"));

  vector<thread> threads;

  for(auto i=0; i<100; i++)
  {
    threads.push_back(thread{[&pool, i]()
    {
      DB::AutoConnection conn(pool);

      for(auto j=0; j<10; j++)
      {
        // Half reading, half writing
        if (i%2)
        {
          auto trans = DB::Transaction{conn};
          ASSERT_TRUE(conn.exec("insert into test (id) values (" +
                                Text::itos(10*i+j) + ")"));
          trans.commit();
        }
        else
        {
          auto result = conn.query("select max(id) from test");
          string value;
          ASSERT_TRUE(result.fetch(value));
        }
      }
    }});
  }

  for(auto& t: threads)
    t.join();

  string value;
  auto result2 = conn.query("select max(id) from test");
  ASSERT_TRUE(result2.fetch(value));
  EXPECT_EQ("999", value);
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    auto level_out = new Log::LevelFilter{chan_out, Log::Level::debug};
    Log::logger.connect(level_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
