//==========================================================================
// ObTools::Merkle: test-sum-tree.cc
//
// Test harness for Merkle Sum tree
//
// Copyright (c) 2024 Paul Clark
//==========================================================================

#include <gtest/gtest.h>
#include "ot-merkle.h"
#include "ot-text.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::Merkle;

struct LeafData
{
  string hash;
  int value;
};

string test_branch_hash_func(const Node<string, int>& left,
                      const Node<string, int>& right)
{
  return "[" + left.get_hash() + "(" + Text::itos(left.get_data()) + "):" +
         right.get_hash() + "(" + Text::itos(right.get_data()) + ")]";
}

string test_branch_single_hash_func(const Node<string, int>& left)
{
  return "[" + left.get_hash() + "(" + Text::itos(left.get_data()) + ")]";
}

int test_leaf_aggregation_func(const LeafData& leaf_data)
{
  return leaf_data.value;
}

int test_branch_aggregation_func(const Node<string, int>& left,
                                 const Node<string, int>& right)
{
  return left.get_data() + right.get_data();
}

string test_leaf_hash_func(const LeafData& leaf_data)
{
  return "<" + leaf_data.hash + "-" + Text::itos(leaf_data.value) + ">";
}

TEST(Tree, TreeValueIsSumOfLeaves)
{
  const auto leaves = vector<LeafData>{
    {"one", 1},
    {"two", 2},
    {"three", 3},
  };
  const auto tree = SumTree<string, LeafData, int,
                            test_leaf_aggregation_func, test_leaf_hash_func,
                            test_branch_hash_func,
                            test_branch_aggregation_func>(leaves);
  EXPECT_EQ(6, tree.get_data());
}

TEST(Tree, TreeOf3ReturnsResultOfHashFunc)
{
  // Has one missing leaf at the end
  const auto leaves = vector<LeafData>{
    {"one", 1},
    {"two", 2},
    {"three", 3},
  };
  const auto tree = SumTree<string, LeafData, int,
                            test_leaf_aggregation_func, test_leaf_hash_func,
                            test_branch_hash_func,
                            test_branch_aggregation_func>(leaves);
  EXPECT_EQ("[[<one-1>(1):<two-2>(2)](3):<three-3>(3)]", tree.get_hash());
}

TEST(Tree, TreeOf5ReturnsResultOfHashFunc)
{
  // Has whole missing branches
  const auto leaves = vector<LeafData>{
    {"one", 1},
    {"two", 2},
    {"three", 3},
    {"four", 4},
    {"five", 5},
  };
  const auto tree = SumTree<string, LeafData, int,
                            test_leaf_aggregation_func, test_leaf_hash_func,
                            test_branch_hash_func,
                            test_branch_aggregation_func>(leaves);
  EXPECT_EQ("[[[<one-1>(1):<two-2>(2)](3):[<three-3>(3):<four-4>(4)](7)](10):<five-5>(5)]", tree.get_hash());
}

TEST(Tree, TreeOf5ReturnsResultOfHashFuncWithCustomSingleHashFunc)
{
  // Has whole missing branches
  const auto leaves = vector<LeafData>{
    {"one", 1},
    {"two", 2},
    {"three", 3},
    {"four", 4},
    {"five", 5},
  };
  const auto tree = SumTree<string, LeafData, int,
                            test_leaf_aggregation_func, test_leaf_hash_func,
                            test_branch_hash_func,
                            test_branch_aggregation_func,
                            test_branch_single_hash_func>(leaves);
  EXPECT_EQ("[[[<one-1>(1):<two-2>(2)](3):[<three-3>(3):<four-4>(4)](7)](10):[[<five-5>(5)](5)](5)]", tree.get_hash());
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
