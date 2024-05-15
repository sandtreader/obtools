//==========================================================================
// ObTools::Merkle: test-tree.cc
//
// Test harness for Merkle tree
//
// Copyright (c) 2024 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-merkle.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::Merkle;

string test_hash_func(const string& left, const string& right)
{
  return left + ":" + right;
}

TEST(Tree, TreeReturnsResultOfHashFunc)
{
  const auto leaves = vector<string>{
    "one",
    "two",
    "three",
    "four",
  };
  const auto tree = Tree<string>(test_hash_func, leaves);
  EXPECT_EQ("one:two:three:four", tree.get_hash());
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
