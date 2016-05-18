//==========================================================================
// ObTools::MT: test-pool.cc
//
// Test harness for thread pools
//
// Copyright (c) 2003-2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-mt.h"

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Tests
TEST(ThreadPoolTest, TestPoolLimit)
{
  class TestThread: public MT::PoolThread
  {
  public:
    int n;

    void run() override
    {
      for (auto i = 0; i < 10; ++i)
      {
        this_thread::sleep_for(chrono::microseconds{10});
      }
    }

    TestThread(MT::PoolReplacer<TestThread>& _rep):
      PoolThread(_rep)
    {}
  };

  MT::ThreadPool<TestThread> pool{1, 10};

  for (auto i = 0; i < 20; ++i)
  {
    TestThread *t = pool.remove();
    if (i < 10)
    {
      EXPECT_NE(nullptr, t);
    }
    else
    {
      EXPECT_EQ(nullptr, t);
      break;
    }
  }
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
