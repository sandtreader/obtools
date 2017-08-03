//==========================================================================
// ObTools::MT: test-queue.cc
//
// Test harness for queue
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-mt.h"

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Tests
TEST(QueueTest, TestLimiting)
{
  MT::Queue<int> q;
  for(auto i=0; i<10; i++) q.send(i);
  ASSERT_EQ(10, q.waiting());
  ASSERT_TRUE(q.limit(5));
  ASSERT_EQ(5, q.waiting());
  EXPECT_EQ(5, q.wait());
  EXPECT_EQ(6, q.wait());
  EXPECT_EQ(7, q.wait());
  EXPECT_EQ(8, q.wait());
  EXPECT_EQ(9, q.wait());
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
