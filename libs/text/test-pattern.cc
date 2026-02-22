//==========================================================================
// ObTools::Text: test-pattern.cc
//
// Test harness for text library pattern match functions
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-text.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(PatternTest, TestSimpleStringMatch)
{
  ASSERT_TRUE(Text::pattern_match("Hello, world!", "Hello, world!"));
}

TEST(PatternTest, TestUncasedSimpleStringMatch)
{
  ASSERT_TRUE(Text::pattern_match("HELLO, world!", "Hello, WORLD!", false));
}

TEST(PatternTest, TestStarPatternMatch)
{
  ASSERT_TRUE(Text::pattern_match("Hello*!", "Hello, world!"));
  ASSERT_FALSE(Text::pattern_match("Hello*!", "Hello, world"));
}

TEST(PatternTest, TestQueryPatternMatch)
{
  ASSERT_TRUE(Text::pattern_match("Hello? world!", "Hello, world!"));
  ASSERT_FALSE(Text::pattern_match("Hello? world!", "Hello world"));
}

TEST(PatternTest, TestSetPatternMatch)
{
  ASSERT_TRUE(Text::pattern_match("[HIJ]ello, world!", "Hello, world!"));
  ASSERT_TRUE(Text::pattern_match("[A-J]ello, world!", "Hello, world!"));
  ASSERT_FALSE(Text::pattern_match("[!A-J]ello, world!", "Hello, world!"));
}

TEST(PatternTest, TestEscapedPatternMatch)
{
  ASSERT_TRUE(Text::pattern_match("Hello\\*, world!", "Hello*, world!"));
  ASSERT_TRUE(Text::pattern_match("Hello\\?, world!", "Hello?, world!"));
  ASSERT_TRUE(Text::pattern_match("Hello\\[, world!", "Hello[, world!"));
  ASSERT_TRUE(Text::pattern_match("Hello\\\\, world!", "Hello\\, world!"));
}

TEST(PatternTest, TestStarCombinedPatternMatch)
{
  ASSERT_TRUE(Text::pattern_match("Hello*[a-e]!", "Hello, world!"));
  ASSERT_FALSE(Text::pattern_match("Hello*[x-z]!", "Hello, world"));
}

TEST(PatternTest, TestStarCapturePatternMatch)
{
  vector<string> captures;
  ASSERT_TRUE(Text::pattern_match("*, *!", "Hello, world!", captures));
  ASSERT_EQ(2, captures.size());
  EXPECT_EQ("Hello", captures[0]);
  EXPECT_EQ("world", captures[1]);
}

TEST(PatternTest, TestStarCombinedCapturePatternMatch)
{
  vector<string> captures;
  ASSERT_TRUE(Text::pattern_match("*[,] *?", "Hello, world!", captures));
  ASSERT_EQ(2, captures.size());
  EXPECT_EQ("Hello", captures[0]);
  EXPECT_EQ("world", captures[1]);
}

TEST(PatternTest, TestStringCaptureOverload)
{
  // Test the string overload with matches vector (pattern.cc line 154-157)
  vector<string> matches;
  ASSERT_TRUE(Text::pattern_match(string("Hello *!"), string("Hello world!"),
                                  matches));
  ASSERT_EQ(1, matches.size());
  EXPECT_EQ("world", matches[0]);
}

TEST(PatternTest, TestUnclosedBracketFallsThrough)
{
  // Unclosed [ has no matching ], triggers goto dft — treat [ as literal
  ASSERT_TRUE(Text::pattern_match("[", "["));
  ASSERT_FALSE(Text::pattern_match("[", "a"));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
