//==========================================================================
// ObTools::MT: test-thread.cc
//
// Test harness for thread functions
//
// Copyright (c) 2003-2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-mt.h"
#include "ot-gen.h"
#include <vector>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Tests
TEST(ThreadTest, TestRuns)
{
  class TestThread: public MT::Thread
  {
  private:
    bool ran;

    void run() override
    {
      ran = true;
    }

  public:
    TestThread():
      ran{false}
    {
      start();
    }

    bool has_run()
    {
      return ran;
    }
  };

  auto threads = vector<TestThread>{10};
  this_thread::sleep_for(chrono::milliseconds{200});
  for (auto& t : threads)
    EXPECT_EQ(true, t.has_run());
}

TEST(ThreadTest, TestLocking)
{
  const auto num_threads = int{100};
  const auto num_iterations = int{1000};

  class TestThread: public MT::Thread
  {
  private:
    MT::Mutex& m;
    unsigned& counter;

    void run() override
    {
      for (auto i = 0; i < num_iterations; ++i)
      {
        MT::Lock lock(m);
        counter = counter + 1; // do not change to ++counter !!
      }
    }

  public:
    TestThread(MT::Mutex &_m, unsigned& _counter):
      m{_m}, counter{_counter}
    {
      start();
    }
  };

  MT::Mutex m;
  auto counter = unsigned{0};
  auto threads = vector<unique_ptr<TestThread>>{};
  for (auto i = 0; i < num_threads; ++i)
    threads.push_back(make_unique<TestThread>(m, counter));
  this_thread::sleep_for(chrono::milliseconds{200});
  EXPECT_EQ(num_threads * num_iterations, counter);
}

TEST(ThreadTest, TestSafeAtPointOfRunning)
{
  class TestThread: public MT::Thread
  {
  private:
    void run() override
    {
      this_thread::sleep_for(chrono::seconds(1));
    }

  public:
    TestThread()
    {
      start();
    }
  };

  ASSERT_NO_THROW(TestThread());
}

TEST(ThreadTest, TestWaitForJoinOnDestruct)
{
  class TestThread: public MT::Thread
  {
  private:
    bool& waited;

    void run() override
    {
      this_thread::sleep_for(chrono::milliseconds(100));
      waited = true;
    }

  public:
    TestThread(bool& _waited):
      waited{_waited}
    {
      start();
    }
  };

  auto waited = false;
  {
    TestThread t{waited};
    this_thread::sleep_for(chrono::milliseconds(10));
  }
  EXPECT_TRUE(waited);
}

TEST(ThreadTest, TestRepeatedStarts)
{
  class TestThread: public MT::Thread
  {
  public:
    int& count;

  private:
    void run() override
    {
      count++;
      // Exits immediately
    }

  public:
    TestThread(int& _count): count(_count)
    {
    }
  };

  auto count = int{0};

  {
    TestThread thread(count);
    for(auto i=0; i<10; i++)
    {
      thread.start();
      this_thread::sleep_for(chrono::milliseconds(20));
    }
  }

  ASSERT_EQ(10, count);
}

TEST(ThreadTest, TestSleepFor)
{
  class TestThread: public MT::Thread
  {
  private:
    void run() override
    {
      sleep_for(chrono::milliseconds(100));
    }
  };

  TestThread thread;
  auto start = chrono::steady_clock::now();
  thread.start();
  this_thread::sleep_for(chrono::milliseconds(10));
  thread.join();
  auto end = chrono::steady_clock::now();
  auto slept_for =
    std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  ASSERT_GE(slept_for, chrono::milliseconds(100));
}

TEST(ThreadTest, TestSleepForInterruptable)
{
  class TestThread: public MT::Thread
  {
  private:
    void run() override
    {
      sleep_for(chrono::seconds(60));
    }
  };

  TestThread thread;
  auto start = chrono::steady_clock::now();
  thread.start();
  this_thread::sleep_for(chrono::milliseconds(10));
  thread.cancel();
  auto end = chrono::steady_clock::now();
  auto slept_for =
    std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  ASSERT_LT(slept_for, chrono::seconds(1));
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
