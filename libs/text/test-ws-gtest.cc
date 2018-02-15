//==========================================================================
// ObTools::Text: test-ws-gtest.cc
//
// GTest harness for text library whitespace functions
//
// Copyright (c) 2015 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-text.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(WSTest, TestRemoveSpace)
{
  ASSERT_EQ("foo", Text::remove_space(" \tf\n\r  oo\r\n "));
}

TEST(WSTest, TestSplitLinesWithBlanks)
{
  string text = "first line\n\nthird line\n";
  vector<string> lines = Text::split_lines(text);
  ASSERT_EQ(3, lines.size());
  EXPECT_EQ("first line", lines[0]);
  EXPECT_EQ("", lines[1]);
  EXPECT_EQ("third line", lines[2]);
}

TEST(WSTest, TestSplitLinesWithoutBlanks)
{
  string text = "first line\n\nthird line";
  vector<string> lines = Text::split_lines(text, true);
  ASSERT_EQ(2, lines.size());
  EXPECT_EQ("first line", lines[0]);
  EXPECT_EQ("third line", lines[1]);
}

TEST(WSTest, TestSplitLinesWithCRLF)
{
  string text = "first line\r\n\r\nthird line";
  vector<string> lines = Text::split_lines(text);
  ASSERT_EQ(3, lines.size());
  EXPECT_EQ("first line", lines[0]);
  EXPECT_EQ("", lines[1]);
  EXPECT_EQ("third line", lines[2]);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
