//==========================================================================
// ObTools::Text: test-base16alpha.cc
//
// GTest harness for text library Base16Alpha functions
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-text.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(Base16AlphaTest, TestEncodeZero)
{
  EXPECT_EQ("b", Text::Base16Alpha::encode(0));
}

TEST(Base16AlphaTest, TestEncodeSmall)
{
  // Alphabet is "bcdghjklmpqrsvwz"
  EXPECT_EQ("c", Text::Base16Alpha::encode(1));
  EXPECT_EQ("z", Text::Base16Alpha::encode(15));
}

TEST(Base16AlphaTest, TestEncodeMultiDigit)
{
  // 16 = 1*16+0 = "cb"
  EXPECT_EQ("cb", Text::Base16Alpha::encode(16));
  // 255 = 15*16+15 = "zz"
  EXPECT_EQ("zz", Text::Base16Alpha::encode(255));
}

TEST(Base16AlphaTest, TestDecodeEmpty)
{
  uint64_t n;
  EXPECT_FALSE(Text::Base16Alpha::decode("", n));
}

TEST(Base16AlphaTest, TestDecodeZero)
{
  uint64_t n;
  EXPECT_TRUE(Text::Base16Alpha::decode("b", n));
  EXPECT_EQ(0, n);
}

TEST(Base16AlphaTest, TestDecodeCaseInsensitive)
{
  uint64_t n1, n2;
  EXPECT_TRUE(Text::Base16Alpha::decode("cb", n1));
  EXPECT_TRUE(Text::Base16Alpha::decode("CB", n2));
  EXPECT_EQ(n1, n2);
}

TEST(Base16AlphaTest, TestDecodeInvalidChar)
{
  uint64_t n;
  // Vowels are not in the alphabet
  EXPECT_FALSE(Text::Base16Alpha::decode("a", n));
  // Numbers are not in the alphabet
  EXPECT_FALSE(Text::Base16Alpha::decode("1", n));
}

TEST(Base16AlphaTest, TestRoundTrip)
{
  uint64_t values[] = {0, 1, 15, 16, 255, 256, 1000, 123456789,
                       0xFFFFFFFFFFFFFFFFULL};
  for (auto v : values)
  {
    string encoded = Text::Base16Alpha::encode(v);
    uint64_t decoded;
    ASSERT_TRUE(Text::Base16Alpha::decode(encoded, decoded))
      << "Failed to decode " << encoded << " (from " << v << ")";
    EXPECT_EQ(v, decoded);
  }
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
