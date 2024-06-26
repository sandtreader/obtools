//==========================================================================
// ObTools::Text: test-const-expr-map.cc
//
// Test harness for Gen library ConstExprMap
//
// Copyright (c) 2024 Paul Clark.  All rights reserved
//==========================================================================

#include "ot-gen.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

static constexpr array<pair<int, int>, 10> data{{
  {0, 19},
  {1, 18},
  {2, 17},
  {3, 16},
  {4, 15},
  {5, 14},
  {6, 13},
  {7, 12},
  {8, 11},
  {9, 10},
}};
static constexpr Gen::ConstExprMap<int, int, 10> map{data};

TEST(ConstExprMapTest, TestLookup)
{
  EXPECT_EQ(13, map.lookup(6));
}

TEST(ConstExprMapTest, TestReverseLookup)
{
  EXPECT_EQ(5, map.reverse_lookup(14));
}

} // anonymous namespace

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
      return RUN_ALL_TESTS();
}
