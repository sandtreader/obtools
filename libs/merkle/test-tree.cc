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

TEST(Tree, TreeReturnsResultOfHashFunc)
{
  const auto leaves = vector<string>{
    "one",
    "two",
    "three"
  };
  const auto tree = Tree<string>(test_hash_func, leaves);
  EXPECT_EQ("one:two:three", tree.get_hash());
}

TEST(Tree, TraverseIsPreorderAcrossWholeTree)
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


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
