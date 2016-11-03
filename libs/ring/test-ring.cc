//==========================================================================
// ObTools::Ring Buffer: test-ring-gtest.cc
//
// GTest tests for ring buffer
//
// Copyright (c) 2014 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-ring.h"

using namespace std;
using namespace ObTools;

TEST(RingBuffer, TestRingBufferSize)
{
  Ring::Buffer<int> buffer(10);
  ASSERT_EQ(10, buffer.size());
}

TEST(RingBuffer, TestEmptyRingBufferHasZeroUsed)
{
  Ring::Buffer<int> buffer(10);
  ASSERT_EQ(0, buffer.used());
}

TEST(RingBuffer, TestRingBufferUsedCountsUp)
{
  Ring::Buffer<int> buffer(10);

  for(int i=1; i<=10; i++)
  {
    buffer.put(i);
    ASSERT_EQ(i, buffer.used());
  }
}

TEST(RingBuffer, TestRingBufferUsedCountsDown)
{
  Ring::Buffer<int> buffer(10);

  for(int i=1; i<=10; i++)
    buffer.put(i);

  for(int i=10; i; i--)
  {
    ASSERT_EQ(i, buffer.used());
    int n;
    ASSERT_TRUE(buffer.get(n));
  }

  ASSERT_EQ(0, buffer.used());
}

TEST(RingBuffer, TestRingBufferUsedWorksAfterWrap)
{
  Ring::Buffer<int> buffer(10);

  for(int i=1; i<=10; i++)
    buffer.put(i);

  ASSERT_EQ(10, buffer.used());

  int n;
  for(int i=0; i<5; i++)
    ASSERT_TRUE(buffer.get(n));

  ASSERT_EQ(5, buffer.used());
  buffer.put(99);
  ASSERT_EQ(6, buffer.used());
  buffer.put(100);
  ASSERT_EQ(7, buffer.used());
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
