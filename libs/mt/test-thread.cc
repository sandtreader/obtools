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
  this_thread::sleep_for(chrono::seconds{1});
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
  this_thread::sleep_for(chrono::seconds{1});
  EXPECT_EQ(num_threads * num_iterations, counter);
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
