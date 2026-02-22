//==========================================================================
// ObTools::Text: test-case.cc
//
// GTest harness for text library case conversion functions
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-text.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(CaseTest, TestToLower)
{
  EXPECT_EQ("hello world", Text::tolower("Hello World"));
  EXPECT_EQ("hello world", Text::tolower("HELLO WORLD"));
  EXPECT_EQ("hello world", Text::tolower("hello world"));
  EXPECT_EQ("123!@#", Text::tolower("123!@#"));
  EXPECT_EQ("", Text::tolower(""));
}

TEST(CaseTest, TestToUpper)
{
  EXPECT_EQ("HELLO WORLD", Text::toupper("Hello World"));
  EXPECT_EQ("HELLO WORLD", Text::toupper("hello world"));
  EXPECT_EQ("HELLO WORLD", Text::toupper("HELLO WORLD"));
  EXPECT_EQ("123!@#", Text::toupper("123!@#"));
  EXPECT_EQ("", Text::toupper(""));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
