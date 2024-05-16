//==========================================================================
// ObTools::Merkle: test-branch.cc
//
// Test harness for Merkle branch
//
// Copyright (c) 2024 Paul Clark.
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

TEST(Branch, BranchReturnsResultOfHashFunc)
{
  unique_ptr<Node<string>> left_leaf = make_unique<Leaf<string>>("left");
  unique_ptr<Node<string>> right_leaf = make_unique<Leaf<string>>("right");
  const auto branch = Branch<string>(test_hash_func, left_leaf, right_leaf);
  EXPECT_EQ("left:right", branch.get_hash());
}

TEST(Branch, TraverseHitsBranchLeftAndRight)
{
  unique_ptr<Node<string>> left_leaf = make_unique<Leaf<string>>("left");
  unique_ptr<Node<string>> right_leaf = make_unique<Leaf<string>>("right");
  const auto branch = Branch<string>(test_hash_func, left_leaf, right_leaf);

  string output;
  branch.traverse_preorder([&output](const Node<string>& node)
  {
    output += node.get_hash() + "/";
  });

  EXPECT_EQ("left:right/left/right/", output);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
