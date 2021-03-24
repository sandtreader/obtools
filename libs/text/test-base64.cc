//==========================================================================
// ObTools::Text: test-base64.cc
//
// GTest harness for text library Base64 functions
// Test cases from Base64.guru
//
// Copyright (c) 2021 Paul Clark.
//==========================================================================

#include "ot-text.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(Base64Test, TestBasicEncode)
{
  Text::Base64 base64;
  EXPECT_EQ("PDw/Pz8+Pg==", base64.encode("<<??\?>>"));  // Escape trigraphs!?
}

TEST(Base64Test, TestBasicDecode)
{
  Text::Base64 base64;
  string text;
  ASSERT_TRUE(base64.decode("PDw/Pz8+Pg==", text));
  EXPECT_EQ("<<??\?>>", text);
}

TEST(Base64URLTest, TestBasicEncode)
{
  Text::Base64URL base64;
  EXPECT_EQ("PDw_Pz8-Pg", base64.encode("<<??\?>>"));
}

TEST(Base64URLTest, TestBasicDecode)
{
  Text::Base64URL base64;
  string text;
  ASSERT_TRUE(base64.decode("PDw_Pz8-Pg", text));
  EXPECT_EQ("<<??\?>>", text);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
