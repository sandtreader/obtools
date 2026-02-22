//==========================================================================
// ObTools::Text: test-split.cc
//
// GTest harness for text library split functions
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-text.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(SplitTest, TestBasicCommaSplit)
{
  auto fields = Text::split("a,b,c");
  ASSERT_EQ(3, fields.size());
  EXPECT_EQ("a", fields[0]);
  EXPECT_EQ("b", fields[1]);
  EXPECT_EQ("c", fields[2]);
}

TEST(SplitTest, TestCustomDelimiter)
{
  auto fields = Text::split("a:b:c", ':');
  ASSERT_EQ(3, fields.size());
  EXPECT_EQ("a", fields[0]);
  EXPECT_EQ("b", fields[1]);
  EXPECT_EQ("c", fields[2]);
}

TEST(SplitTest, TestCanonicalisesWhitespace)
{
  auto fields = Text::split("  a  , b   c  , d ");
  ASSERT_EQ(3, fields.size());
  EXPECT_EQ("a", fields[0]);
  EXPECT_EQ("b c", fields[1]);
  EXPECT_EQ("d", fields[2]);
}

TEST(SplitTest, TestNoCanonicalize)
{
  auto fields = Text::split("  a  , b  ", ',', false);
  ASSERT_EQ(2, fields.size());
  EXPECT_EQ("  a  ", fields[0]);
  EXPECT_EQ(" b  ", fields[1]);
}

TEST(SplitTest, TestNoDelimiter)
{
  auto fields = Text::split("hello world");
  ASSERT_EQ(1, fields.size());
  EXPECT_EQ("hello world", fields[0]);
}

TEST(SplitTest, TestMaxFields)
{
  auto fields = Text::split("a,b,c,d,e", ',', true, 3);
  ASSERT_EQ(3, fields.size());
  EXPECT_EQ("a", fields[0]);
  EXPECT_EQ("b", fields[1]);
  EXPECT_EQ("c,d,e", fields[2]);
}

TEST(SplitTest, TestEmptyString)
{
  auto fields = Text::split("");
  ASSERT_EQ(1, fields.size());
  EXPECT_EQ("", fields[0]);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
