//==========================================================================
// ObTools::G2D: test-point.cc
//
// Test harness for point functions
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-g2d.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::G2D;

TEST(Points, TestDefaultPointIsOrigin)
{
  Point p;
  EXPECT_EQ(0, p.x);
  EXPECT_EQ(0, p.y);
}

TEST(Points, TestDefaultPointConstructor)
{
  Point p(1,2);
  EXPECT_EQ(1, p.x);
  EXPECT_EQ(2, p.y);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
