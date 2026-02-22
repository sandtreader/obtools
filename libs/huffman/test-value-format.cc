//==========================================================================
// ObTools::Huffman: test-value-format.cc
//
// Test harness for Huffman value operator<< and special values
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-huffman.h"
#include <gtest/gtest.h>
#include <sstream>

namespace {

using namespace std;
using namespace ObTools;

TEST(ValueFormatTest, TestOutputRegularValue)
{
  Huffman::Value v('A');
  ostringstream oss;
  oss << v;
  EXPECT_EQ("A", oss.str());
}

TEST(ValueFormatTest, TestOutputSpecialStart)
{
  Huffman::Value v(Huffman::Value::Special::start);
  ostringstream oss;
  oss << v;
  EXPECT_EQ("START", oss.str());
}

TEST(ValueFormatTest, TestOutputSpecialStop)
{
  Huffman::Value v(Huffman::Value::Special::stop);
  ostringstream oss;
  oss << v;
  EXPECT_EQ("STOP", oss.str());
}

TEST(ValueFormatTest, TestOutputSpecialEscape)
{
  Huffman::Value v(Huffman::Value::Special::escape);
  ostringstream oss;
  oss << v;
  EXPECT_EQ("ESCAPE", oss.str());
}

TEST(ValueFormatTest, TestOutputSpecialNone)
{
  Huffman::Value v(Huffman::Value::Special::none);
  ostringstream oss;
  oss << v;
  EXPECT_EQ("NONE", oss.str());
}

TEST(ValueFormatTest, TestDefaultValueIsInvalid)
{
  Huffman::Value v;
  EXPECT_TRUE(!v);
}

TEST(ValueFormatTest, TestRegularValueIsValid)
{
  Huffman::Value v('X');
  EXPECT_FALSE(!v);
}

TEST(ValueFormatTest, TestSpecialValueIsSpecial)
{
  Huffman::Value v(Huffman::Value::Special::start);
  EXPECT_TRUE(v.is_special());
  EXPECT_EQ(Huffman::Value::Special::start, v.get_special_value());
}

TEST(ValueFormatTest, TestRegularValueNotSpecial)
{
  Huffman::Value v('A');
  EXPECT_FALSE(v.is_special());
  EXPECT_EQ('A', v.get_value());
}

TEST(ValueFormatTest, TestValueEquality)
{
  Huffman::Value v1('A');
  Huffman::Value v2('A');
  Huffman::Value v3('B');
  EXPECT_TRUE(v1 == v2);
  EXPECT_FALSE(v1 == v3);
}

TEST(ValueFormatTest, TestValueLessThan)
{
  // Regular < regular
  Huffman::Value v1('A');
  Huffman::Value v2('B');
  EXPECT_TRUE(v1 < v2);
  EXPECT_FALSE(v2 < v1);

  // Regular < special (regular should be less than special)
  Huffman::Value vs(Huffman::Value::Special::start);
  EXPECT_TRUE(v1 < vs);
  EXPECT_FALSE(vs < v1);

  // Special < special (by enum value)
  Huffman::Value vs2(Huffman::Value::Special::stop);
  EXPECT_TRUE(vs < vs2);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
