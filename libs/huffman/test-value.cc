//==========================================================================
// ObTools::Huffman: test-value.cc
//
// Test harness for Huffman Value class
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-huffman.h"

using namespace std;
using namespace ObTools;

TEST(TreeValue, TestNormalIsNotSpecial)
{
  Huffman::Value a('a');
  ASSERT_FALSE(a.is_special());
}

TEST(TreeValue, TestSpecialIsNotSpecial)
{
  Huffman::Value a(Huffman::Value::START);
  ASSERT_TRUE(a.is_special());
}

TEST(TreeValue, TestNormalLessThanSpecial)
{
  Huffman::Value a('a');
  Huffman::Value b(Huffman::Value::NONE);
  ASSERT_LT(a, b);
}

TEST(TreeValue, TestNormalLessThan)
{
  Huffman::Value a('a');
  Huffman::Value b('b');
  ASSERT_LT(a, b);
}

TEST(TreeValue, TestSpecialLessThan)
{
  Huffman::Value a(Huffman::Value::START);
  Huffman::Value b(Huffman::Value::STOP);
  ASSERT_LT(a, b);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
