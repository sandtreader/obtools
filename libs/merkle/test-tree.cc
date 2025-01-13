//==========================================================================
// ObTools::Merkle: test-tree.cc
//
// Test harness for Merkle tree
//
// Copyright (c) 2024 Paul Clark
//==========================================================================

#include <gtest/gtest.h>
#include "ot-merkle.h"
#include "ot-text.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::Merkle;

string test_hash_func(const Node<string>& left, const Node<string>& right)
{
  return left.get_hash() + ":" + right.get_hash();
}

TEST(Tree, TreeOf3ReturnsResultOfHashFunc)
{
  // Has one missing leaf at the end
  const auto leaves = vector<string>{
    "one",
    "two",
    "three"
  };
  const auto tree = Tree<string, test_hash_func>(leaves);
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
  const auto tree = Tree<string, test_hash_func>(leaves);
  EXPECT_EQ("one:two:three:four:five", tree.get_hash());
}

TEST(Tree, TraverseIsPreorderAcrossWholeTreeOf3)
{
  const auto leaves = vector<string>{
    "one",
    "two",
    "three"
  };
  const auto tree = Tree<string, test_hash_func>(leaves);

  string output;
  tree.traverse_preorder([&output](const Node<string>& node)
  {
    output += node.get_hash() + "(" + Text::itos(node.index) + ")/";
  });

  EXPECT_EQ("one:two:three(0)/one:two(1)/one(3)/two(4)/three(2)/three(5)/",
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
  const auto tree = Tree<string, test_hash_func>(leaves);

  string output;
  tree.traverse_preorder([&output](const Node<string>& node)
  {
    output += node.get_hash() + "(" + Text::itos(node.index) + ")/";
  });

  EXPECT_EQ("one:two:three:four:five(0)/one:two:three:four(1)/one:two(3)/one(7)/two(8)/three:four(4)/three(9)/four(10)/five(2)/five(5)/five(11)/",
            output);
}

TEST(Tree, TraverseIsBreadthFirstAcrossWholeTreeOf5)
{
  const auto leaves = vector<string>{
    "one",
    "two",
    "three",
    "four",
    "five"
  };
  const auto tree = Tree<string, test_hash_func>(leaves);

  string output;
  tree.traverse_breadth_first([&output](const Node<string>& node)
  {
    output += node.get_hash() + "(" + Text::itos(node.index) + ")/";
  });

  EXPECT_EQ("one:two:three:four:five(0)/one:two:three:four(1)/five(2)/one:two(3)/three:four(4)/five(5)/one(7)/two(8)/three(9)/four(10)/five(11)/",
            output);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
