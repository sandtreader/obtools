//==========================================================================
// ObTools::DB: test-pool.cc
//
// Test harness for database pool
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-log.h"
#include "ot-db.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::DB;

// Fake connection
class FakeConnection: public Connection
{
  static int counter;
  int serial;

public:
  FakeConnection(): serial(++counter)
  {
    Log::Summary log;
    log << "Creating connection " << serial << endl;
  }
  virtual explicit operator bool() { return true; }
  virtual bool exec(const string&) { return true; }
  virtual Result query(const string&) { return Result(); }
  virtual Statement prepare(const string&) { return Statement(); }
  virtual uint64_t get_last_insert_id() { return 0; }

  ~FakeConnection()
  {
    Log::Summary log;
    log << "Destroying connection " << serial << endl;
  }
};

int FakeConnection::counter = 0;

// Fake connection factory
class FakeConnectionFactory: public ConnectionFactory
{
public:
  FakeConnection *create_connection() { return new FakeConnection(); }
};

TEST(DatabasePool, TestSimpleClaimRelease)
{
  FakeConnectionFactory factory;
  ConnectionPool pool(factory, 1, 6, Time::Duration(5));
  EXPECT_EQ(1, pool.num_connections());
  EXPECT_EQ(1, pool.num_available());
  EXPECT_EQ(0, pool.num_in_use());

  Connection *conn = pool.claim();

  EXPECT_EQ(1, pool.num_connections());
  EXPECT_EQ(0, pool.num_available());
  EXPECT_EQ(1, pool.num_in_use());

  ASSERT_TRUE(!!conn);
  pool.release(conn);

  EXPECT_EQ(1, pool.num_connections());
  EXPECT_EQ(1, pool.num_available());
  EXPECT_EQ(0, pool.num_in_use());
}

TEST(DatabasePool, TestDoubleReleaseDoesntBreak)
{
  FakeConnectionFactory factory;
  ConnectionPool pool(factory, 1, 6, Time::Duration(5));
  EXPECT_EQ(1, pool.num_connections());
  EXPECT_EQ(1, pool.num_available());
  EXPECT_EQ(0, pool.num_in_use());

  Connection *conn = pool.claim();
  ASSERT_TRUE(!!conn);
  pool.release(conn);

  EXPECT_EQ(1, pool.num_connections());
  EXPECT_EQ(1, pool.num_available());
  EXPECT_EQ(0, pool.num_in_use());

  pool.release(conn);  // Will complain but no bad effect

  EXPECT_EQ(1, pool.num_connections());
  EXPECT_EQ(1, pool.num_available());
  EXPECT_EQ(0, pool.num_in_use());

}

TEST(DatabasePool, TestConnectionNotRecreatedOnRelease)
{
  FakeConnectionFactory factory;
  ConnectionPool pool(factory, 1, 6, Time::Duration(5));
  Connection *conn = pool.claim();
  ASSERT_TRUE(!!conn);
  pool.release(conn);

  Connection *conn2 = pool.claim();
  ASSERT_EQ(conn, conn2);
  pool.release(conn2);
}

TEST(DatabasePool, TestSecondConnectionWithOverlap)
{
  FakeConnectionFactory factory;
  ConnectionPool pool(factory, 1, 6, Time::Duration(5));
  Connection *conn1 = pool.claim();
  ASSERT_TRUE(!!conn1);

  EXPECT_EQ(1, pool.num_connections());
  EXPECT_EQ(0, pool.num_available());
  EXPECT_EQ(1, pool.num_in_use());

  Connection *conn2 = pool.claim();
  ASSERT_TRUE(!!conn2);
  ASSERT_NE(conn1, conn2);

  EXPECT_EQ(2, pool.num_connections());
  EXPECT_EQ(0, pool.num_available());
  EXPECT_EQ(2, pool.num_in_use());

  pool.release(conn1);

  EXPECT_EQ(2, pool.num_connections());
  EXPECT_EQ(1, pool.num_available());
  EXPECT_EQ(1, pool.num_in_use());

  pool.release(conn2);

  EXPECT_EQ(2, pool.num_connections());
  EXPECT_EQ(2, pool.num_available());
  EXPECT_EQ(0, pool.num_in_use());
}

TEST(DatabasePool, TestReapIdleConnectionsToMinimum)
{
  FakeConnectionFactory factory;
  ConnectionPool pool(factory, 1, 6, Time::Duration(0.3));
  pool.set_reap_interval(Time::Duration(0.1));
  Connection *conn1 = pool.claim();
  ASSERT_TRUE(!!conn1);

  Connection *conn2 = pool.claim();
  ASSERT_TRUE(!!conn2);
  ASSERT_NE(conn1, conn2);

  pool.release(conn1);

  EXPECT_EQ(2, pool.num_connections());
  EXPECT_EQ(1, pool.num_available());
  EXPECT_EQ(1, pool.num_in_use());

  // Sleep to allow pool to reap
  sleep(1);

  EXPECT_EQ(1, pool.num_connections());
  EXPECT_EQ(0, pool.num_available());
  EXPECT_EQ(1, pool.num_in_use());

  pool.release(conn2);

  EXPECT_EQ(1, pool.num_connections());
  EXPECT_EQ(1, pool.num_available());
  EXPECT_EQ(0, pool.num_in_use());
}

TEST(DatabasePool, TestBlockOnPoolEmpty)
{
  FakeConnectionFactory factory;
  ConnectionPool pool(factory, 1, 1, Time::Duration(0.5));
  Connection *conn1 = pool.claim();
  ASSERT_TRUE(!!conn1);

  // Start separate thread which will hold and then release the connection
  thread thread1(
    [&pool, &conn1]
    {
      sleep(1);
      pool.release(conn1);
    });

  Connection *conn2 = pool.claim();
  ASSERT_TRUE(!!conn2);
  thread1.join();
}

TEST(DatabasePool, TestTimeoutOfBlockedClaims)
{
  FakeConnectionFactory factory;
  ConnectionPool pool(factory, 1, 1, Time::Duration(0.5));
  pool.set_reap_interval(Time::Duration(0.1));
  pool.set_claim_timeout(Time::Duration(1));
  Connection *conn1 = pool.claim();
  ASSERT_TRUE(!!conn1);

  Connection *conn2 = pool.claim();
  ASSERT_TRUE(!conn2);  // Should fail after 1 second
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
