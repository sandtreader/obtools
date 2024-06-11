//==========================================================================
// ObTools::Text: test-base58.cc
//
// GTest harness for text library Base58 functions
// Test cases from draft-msporny-base58-03
// https://digitalbazaar.github.io/base58-spec/
//
// Copyright (c) 2021 Paul Clark.
//==========================================================================

#include "ot-text.h"
#include <gtest/gtest.h>
#include <algorithm>

namespace {

using namespace std;
using namespace ObTools;

TEST(Base58Test, hello_world_encode)
{
  Text::Base58 base58;
  string text("Hello World!");
  vector<byte> binary;
  for(auto c: text) binary.push_back((byte)c);
  EXPECT_EQ("2NEpo7TZRRrLZSi2U", base58.encode(binary));
}

TEST(Base58Test, quick_brown_fox_encode)
{
  Text::Base58 base58;
  string text("The quick brown fox jumps over the lazy dog.");
  vector<byte> binary;
  for(auto c: text) binary.push_back((byte)c);
  EXPECT_EQ("USm3fpXnKG5EUBx2ndxBDMPVciP5hGey2Jh4NDv6gmeo1LkMeiKrLJUUBk6Z",
            base58.encode(binary));
}

TEST(Base58Test, leading_zeros_encode)
{
  Text::Base58 base58;
  vector<byte> binary;
  Text::xtob("0000287fb4cd", binary);
  EXPECT_EQ("11233QC4", base58.encode(binary));
}

TEST(Base58Test, hello_world_decode)
{
  Text::Base58 base58;
  string encoding("2NEpo7TZRRrLZSi2U");
  vector<byte> binary;
  ASSERT_TRUE(base58.decode(encoding, binary));
  string text;
  for(auto b: binary) text += (char)b;
  EXPECT_EQ("Hello World!", text);
}

TEST(Base58Test, quick_brown_fox_decode)
{
  Text::Base58 base58;
  string encoding("USm3fpXnKG5EUBx2ndxBDMPVciP5hGey2Jh4NDv6gmeo1LkMeiKrLJUUBk6Z");
  vector<byte> binary;
  ASSERT_TRUE(base58.decode(encoding, binary));
  string text;
  for(auto b: binary) text += (char)b;
  EXPECT_EQ("The quick brown fox jumps over the lazy dog.", text);
}

TEST(Base58Test, leading_zeros_decode)
{
  Text::Base58 base58;
  string encoding("11233QC4");
  vector<byte> binary;
  ASSERT_TRUE(base58.decode(encoding, binary));
  EXPECT_EQ("0000287fb4cd", Text::btox(binary));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
