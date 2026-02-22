//==========================================================================
// ObTools::Text: test-base36.cc
//
// GTest harness for text library Base36 functions
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-text.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(Base36Test, TestEncodeZero)
{
  EXPECT_EQ("0", Text::Base36::encode(0));
}

TEST(Base36Test, TestEncodeSingleDigit)
{
  EXPECT_EQ("1", Text::Base36::encode(1));
  EXPECT_EQ("9", Text::Base36::encode(9));
  EXPECT_EQ("a", Text::Base36::encode(10));
  EXPECT_EQ("z", Text::Base36::encode(35));
}

TEST(Base36Test, TestEncodeMultiDigit)
{
  EXPECT_EQ("10", Text::Base36::encode(36));
  EXPECT_EQ("2s", Text::Base36::encode(100));
  EXPECT_EQ("rs", Text::Base36::encode(1000));
}

TEST(Base36Test, TestDecodeEmpty)
{
  uint64_t n;
  EXPECT_FALSE(Text::Base36::decode("", n));
}

TEST(Base36Test, TestDecodeZero)
{
  uint64_t n;
  EXPECT_TRUE(Text::Base36::decode("0", n));
  EXPECT_EQ(0, n);
}

TEST(Base36Test, TestDecodeSingleDigit)
{
  uint64_t n;
  EXPECT_TRUE(Text::Base36::decode("a", n));
  EXPECT_EQ(10, n);
  EXPECT_TRUE(Text::Base36::decode("z", n));
  EXPECT_EQ(35, n);
}

TEST(Base36Test, TestDecodeCaseInsensitive)
{
  uint64_t n1, n2;
  EXPECT_TRUE(Text::Base36::decode("abc", n1));
  EXPECT_TRUE(Text::Base36::decode("ABC", n2));
  EXPECT_EQ(n1, n2);
}

TEST(Base36Test, TestDecodeInvalidChar)
{
  uint64_t n;
  EXPECT_FALSE(Text::Base36::decode("!@#", n));
}

TEST(Base36Test, TestRoundTrip)
{
  uint64_t values[] = {0, 1, 35, 36, 100, 1000, 123456789, 0xFFFFFFFFFFFFFFFFULL};
  for (auto v : values)
  {
    string encoded = Text::Base36::encode(v);
    uint64_t decoded;
    ASSERT_TRUE(Text::Base36::decode(encoded, decoded));
    EXPECT_EQ(v, decoded);
  }
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
