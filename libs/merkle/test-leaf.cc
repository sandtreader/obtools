//==========================================================================
// ObTools::Merkle: test-leaf.cc
//
// Test harness for Merkle Leaf
//
// Copyright (c) 2024 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-merkle.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::Merkle;

TEST(Leaf, LeafReturnsGivenHash)
{
  const auto hash = string{"testHash"};
  const auto leaf = Leaf<string>{hash};
  EXPECT_EQ(hash, leaf.get_hash());
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
