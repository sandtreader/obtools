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

TEST(WSTest, TestStripBlankLinesAllBlank)
{
  EXPECT_EQ("", Text::strip_blank_lines("  \n\n  \n"));
}

TEST(WSTest, TestStripBlankLinesNoNewlines)
{
  EXPECT_EQ("hello", Text::strip_blank_lines("hello"));
}

TEST(WSTest, TestStripBlankLinesLeadingBlank)
{
  EXPECT_EQ("hello\n", Text::strip_blank_lines("\nhello\n"));
}

TEST(WSTest, TestStripBlankLinesTrailingBlank)
{
  EXPECT_EQ("hello\n\n", Text::strip_blank_lines("hello\n\n"));
}

TEST(WSTest, TestStripBlankLinesFirstLineNotBlank)
{
  EXPECT_EQ("hello\nworld\n", Text::strip_blank_lines("hello\nworld\n"));
}

TEST(WSTest, TestStripBlankLinesNoTrailingNewline)
{
  // Last newline is before last content, so we hit substr(start) path
  EXPECT_EQ("hello\nworld", Text::strip_blank_lines("\nhello\nworld"));
}

TEST(WSTest, TestCondenseBlankLines)
{
  string input = "\n\nfirst\n\n\n\nsecond\n\n\n";
  string result = Text::condense_blank_lines(input);
  EXPECT_EQ("first\n\nsecond\n", result);
}

TEST(WSTest, TestCondenseBlankLinesNoBlankLines)
{
  EXPECT_EQ("first\nsecond\n", Text::condense_blank_lines("first\nsecond\n"));
}

TEST(WSTest, TestCondenseBlankLinesWithTabs)
{
  EXPECT_EQ("first\n\nsecond\n",
            Text::condense_blank_lines("\t\nfirst\n\t\n\nsecond\n"));
}

TEST(WSTest, TestGetCommonIndent)
{
  EXPECT_EQ(2, Text::get_common_indent("  hello\n  world"));
  EXPECT_EQ(0, Text::get_common_indent("hello\n  world"));
  EXPECT_EQ(4, Text::get_common_indent("    a\n    b\n"));
}

TEST(WSTest, TestGetCommonIndentWithTabs)
{
  EXPECT_EQ(8, Text::get_common_indent("\thello\n\tworld"));
}

TEST(WSTest, TestGetCommonIndentBlankLines)
{
  EXPECT_EQ(2, Text::get_common_indent("  hello\n\n  world"));
}

TEST(WSTest, TestRemoveIndent)
{
  EXPECT_EQ("hello\nworld\n", Text::remove_indent("  hello\n  world\n", 2));
}

TEST(WSTest, TestRemoveIndentWithTabs)
{
  // Tab counts as 8 spaces; removing 4 spaces of indent still consumes the tab
  string result = Text::remove_indent("\thello\n", 4);
  EXPECT_EQ("hello\n", result);
}

TEST(WSTest, TestRemoveIndentPartial)
{
  EXPECT_EQ("  hello\n", Text::remove_indent("    hello\n", 2));
}

TEST(WSTest, TestCanonicaliseSpace)
{
  EXPECT_EQ("hello world", Text::canonicalise_space("  hello   world  "));
  EXPECT_EQ("a b c", Text::canonicalise_space("  a  b  c  "));
  EXPECT_EQ("", Text::canonicalise_space("   "));
  EXPECT_EQ("x", Text::canonicalise_space("x"));
}

TEST(WSTest, TestCanonicaliseSpaceWithTabs)
{
  EXPECT_EQ("hello world", Text::canonicalise_space("\thello\t\tworld\t"));
}

TEST(WSTest, TestRemoveWord)
{
  string text = "hello world foo";
  EXPECT_EQ("hello", Text::remove_word(text));
  EXPECT_EQ("world foo", text);
  EXPECT_EQ("world", Text::remove_word(text));
  EXPECT_EQ("foo", text);
  EXPECT_EQ("foo", Text::remove_word(text));
  EXPECT_EQ("", text);
}

TEST(WSTest, TestRemoveWordEmpty)
{
  string text = "";
  EXPECT_EQ("", Text::remove_word(text));
  EXPECT_EQ("", text);
}

TEST(WSTest, TestRemoveLine)
{
  string text = "first\nsecond\nthird";
  EXPECT_EQ("first", Text::remove_line(text));
  EXPECT_EQ("second\nthird", text);
  EXPECT_EQ("second", Text::remove_line(text));
  EXPECT_EQ("third", text);
  EXPECT_EQ("third", Text::remove_line(text));
  EXPECT_EQ("", text);
}

TEST(WSTest, TestRemoveLineNoNewline)
{
  string text = "only";
  EXPECT_EQ("only", Text::remove_line(text));
  EXPECT_EQ("", text);
}

TEST(WSTest, TestSplitWords)
{
  auto words = Text::split_words("  hello   world  ");
  ASSERT_EQ(2, words.size());
  EXPECT_EQ("hello", words[0]);
  EXPECT_EQ("world", words[1]);
}

TEST(WSTest, TestSplitWordsEmpty)
{
  auto words = Text::split_words("");
  EXPECT_TRUE(words.empty());
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
