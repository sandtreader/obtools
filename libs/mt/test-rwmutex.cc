//==========================================================================
// ObTools::MT: test-rwmutex.cc
//
// Test harness for reader/writer mutex functions
//
// Copyright (c) 2007-2016 Paul Clark.  All rights reserved
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
TEST(RWMutexTest, TestSafe)
{
  static MT::RWMutex m;
  static auto bad = false;
  static auto reads = int{0};
  static auto writes = int{0};
  static auto bad_reads = int{0};
  static auto bad_writes = int{0};

  class ReaderThread: public MT::Thread
  {
  private:
    void run() override
    {
      while (running)
      {
        MT::RWReadLock lock{m};
        if (bad)
          ++bad_reads;
        ++reads;
      }
    }
  public:
    ReaderThread()
    {
      start();
    }
  };

  class WriterThread: public MT::Thread
  {
  private:
    void run() override
    {
      while (running)
      {
        {
          MT::RWWriteLock lock{m};
          if (bad)
            ++bad_writes;

          bad = true;
          this_thread::sleep_for(chrono::microseconds{10});
          bad = false;
        }
        ++writes;
        this_thread::sleep_for(chrono::microseconds{10});
      }
    }
  public:
    WriterThread()
    {
      start();
    }
  };

  {
    auto readers = vector<ReaderThread>{2};
    auto writers = vector<WriterThread>{2};
    this_thread::sleep_for(chrono::seconds{1});
  }
  EXPECT_GT(reads, 5000);
  EXPECT_GT(writes, 5000);
  EXPECT_EQ(0, bad_reads);
  EXPECT_EQ(0, bad_writes);
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
