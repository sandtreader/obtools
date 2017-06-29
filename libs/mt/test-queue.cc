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
  ASSERT_EQ(10, q.count());
  ASSERT_TRUE(q.limit(5));
  ASSERT_EQ(5, q.count());
  EXPECT_EQ(5, q.wait());
  EXPECT_EQ(6, q.wait());
  EXPECT_EQ(7, q.wait());
  EXPECT_EQ(8, q.wait());
  EXPECT_EQ(9, q.wait());
}

TEST(QueueTest, TestContainsValue)
{
  MT::Queue<int> q;
  for(auto i=0; i<10; i++) q.send(i);
  ASSERT_TRUE(q.contains(5));
  ASSERT_FALSE(q.contains(11));
}

TEST(QueueTest, TestContainsPointer)
{
  int one=1;
  int two=2;
  MT::Queue<int *> q;
  q.send(&one);
  q.send(&two);
  int two2 = 2;
  int three2 = 3;
  ASSERT_TRUE(q.contains_ptr(&two2));
  ASSERT_FALSE(q.contains_ptr(&three2));
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
