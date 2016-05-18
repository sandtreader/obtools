//==========================================================================
// ObTools::MT: test-dqueue.cc
//
// Test harness for data queue
//
// Copyright (c) 2010-2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-mt.h"

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Tests
TEST(DataQueueTest, TestReceivingFromMultipleWriters)
{
  static MT::DataQueue dq;
  static const auto num_writers = int{10};
  static const auto num_sends = int{1000};
  static const unsigned char message[] = "Hello, world!";

  class WriterThread: public ObTools::MT::Thread
  {
  private:
    void run() override
    {
      for (auto i = 0; i < num_sends; ++i)
      {
        dq.write(message, sizeof(message));
      }
    }

  public:
    WriterThread()
    {
      start();
    }
  };

  vector<WriterThread> writers{num_writers};
  this_thread::sleep_for(chrono::seconds{1});
  dq.close();

  vector<unsigned char> received(num_writers * num_sends * sizeof(message));
  auto read = dq.read(&received[0], received.size(), true);

  ASSERT_EQ(num_writers * num_sends * sizeof(message), read);
  for (auto i = 0u; i < num_writers; ++i)
  {
    for (auto j = 0u; j < num_sends; ++j)
    {
      for (auto k = 0u; k < sizeof(message); ++k)
      {
        EXPECT_EQ(message[k], received[i * sizeof(message) +
                                       j * sizeof(message) + k]);
      }
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
