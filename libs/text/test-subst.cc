//==========================================================================
// ObTools::Text: test-subst.cc
//
// GTest harness for text library substitution functions
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-text.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(SubstTest, TestSimpleReplace)
{
  EXPECT_EQ("hello world", Text::subst("hello there", "there", "world"));
}

TEST(SubstTest, TestMultipleReplace)
{
  EXPECT_EQ("b-b-b", Text::subst("a-a-a", "a", "b"));
}

TEST(SubstTest, TestNoMatch)
{
  EXPECT_EQ("hello", Text::subst("hello", "xyz", "abc"));
}

TEST(SubstTest, TestDeleteByEmptyReplacement)
{
  EXPECT_EQ("hllo", Text::subst("hello", "e", ""));
}

TEST(SubstTest, TestReplacementContainsOriginal)
{
  // Replacing "a" with "aa" should not loop infinitely;
  // the search advances past each replacement
  EXPECT_EQ("aab", Text::subst("ab", "a", "aa"));
}

TEST(SubstTest, TestEmptyInput)
{
  EXPECT_EQ("", Text::subst("", "a", "b"));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
