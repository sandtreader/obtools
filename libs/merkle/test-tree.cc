//==========================================================================
// ObTools::Merkle: test-tree.cc
//
// Test harness for Merkle tree
//
// Copyright (c) 2024 Paul Clark
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

TEST(Tree, TreeOf3ReturnsResultOfHashFunc)
{
  // Has one missing leaf at the end
  const auto leaves = vector<string>{
    "one",
    "two",
    "three"
  };
  const auto tree = Tree<string>(test_hash_func, leaves);
  EXPECT_EQ("one:two:three", tree.get_hash());
}

TEST(Tree, TreeOf5ReturnsResultOfHashFunc)
{
  // Has whole missing branches
  const auto leaves = vector<string>{
    "one",
    "two",
    "three",
    "four",
    "five"
  };
  const auto tree = Tree<string>(test_hash_func, leaves);
  EXPECT_EQ("one:two:three:four:five", tree.get_hash());
}

TEST(Tree, TraverseIsPreorderAcrossWholeTreeOf3)
{
  const auto leaves = vector<string>{
    "one",
    "two",
    "three"
  };
  const auto tree = Tree<string>(test_hash_func, leaves);

  string output;
  tree.traverse_preorder([&output](const Node<string>& node)
  {
    output += node.get_hash() + "/";
  });

  EXPECT_EQ("one:two:three/one:two/one/two/three/three/",
            output);
}

TEST(Tree, TraverseIsPreorderAcrossWholeTreeOf5)
{
  const auto leaves = vector<string>{
    "one",
    "two",
    "three",
    "four",
    "five"
  };
  const auto tree = Tree<string>(test_hash_func, leaves);

  string output;
  tree.traverse_preorder([&output](const Node<string>& node)
  {
    output += node.get_hash() + "/";
  });

  EXPECT_EQ("one:two:three:four:five/one:two:three:four/one:two/one/two/three:four/three/four/five/five/five/",
            output);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
