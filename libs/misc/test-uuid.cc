//==========================================================================
// ObTools::Text: test-uuid.cc
//
// Test harness for text library UUID class
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-misc.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;
using namespace ObTools::Misc;

TEST(UUIDTest, TestBasicConstruction)
{
  const auto actual = UUID{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                           0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
  const auto expected = array<unsigned char, 16>{{0x01, 0x02, 0x03, 0x04,
                                                  0x05, 0x06, 0x07, 0x08,
                                                  0x09, 0x0a, 0x0b, 0x0c,
                                                  0x0d, 0x0e, 0x0f, 0x10}};
  EXPECT_EQ(expected, actual);
}

TEST(UUIDTest, TestHexStringConstruction)
{
  const auto actual = UUID{"0102030405060708090a0b0c0d0e0f10"};
  const auto expected = array<unsigned char, 16>{{0x01, 0x02, 0x03, 0x04,
                                                  0x05, 0x06, 0x07, 0x08,
                                                  0x09, 0x0a, 0x0b, 0x0c,
                                                  0x0d, 0x0e, 0x0f, 0x10}};
  EXPECT_EQ(expected, actual);
}

TEST(UUIDTest, TestUUIDStringConstruction)
{
  const auto actual = UUID{"01020304-0506-0708-090a-0b0c0d0e0f10"};
  const auto expected = array<unsigned char, 16>{{0x01, 0x02, 0x03, 0x04,
                                                  0x05, 0x06, 0x07, 0x08,
                                                  0x09, 0x0a, 0x0b, 0x0c,
                                                  0x0d, 0x0e, 0x0f, 0x10}};
  EXPECT_EQ(expected, actual);
}

TEST(UUIDTest, TestAsHexString)
{
  const auto actual = UUID{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                           0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
  const auto expected = string{"0102030405060708090a0b0c0d0e0f10"};
  EXPECT_EQ(expected, actual.get_hex_str());
}

TEST(UUIDTest, TestAsString)
{
  const auto actual = UUID{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                           0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
  const auto expected = string{"01020304-0506-0708-090a-0b0c0d0e0f10"};
  EXPECT_EQ(expected, actual.get_str());
}

TEST(UUIDTest, TestAsBase64String)
{
  const auto actual = UUID{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                           0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
  const auto expected = string{"AQIDBAUGBwgJCgsMDQ4PEA=="};
  EXPECT_EQ(expected, actual.get_base64_str());
}

TEST(UUIDTest, TestRandomisation)
{
  auto uuid = UUID{};
  auto previous = vector<UUID>{};
  for (auto i = 0; i < 1000; ++i)
  {
    previous.push_back(uuid);
    uuid.randomise();
    for (const auto& p: previous)
      ASSERT_NE(p, uuid);
  }
}

TEST(UUIDTest, TestValidity)
{
  auto uuid = UUID{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  EXPECT_EQ(true, !uuid);
  uuid.randomise();
  EXPECT_EQ(false, !uuid);
}

TEST(UUIDTest, TestDefaultConstructorIsAllZeros)
{
  UUID uuid;
  EXPECT_FALSE(static_cast<bool>(uuid));
}

TEST(UUIDTest, TestRawStringConstruction)
{
  // 16-byte raw string — not hex or UUID format
  string raw(16, '\x01');
  UUID uuid(raw);
  for (int i = 0; i < 16; i++)
    EXPECT_EQ(0x01, uuid[i]);
}

TEST(UUIDTest, TestBadLengthStringConstruction)
{
  // Length != 16, 32, or 36 falls through to default (zeros)
  UUID uuid("short");
  EXPECT_FALSE(static_cast<bool>(uuid));
}

TEST(UUIDTest, TestStreamOperator)
{
  UUID uuid{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
  ostringstream oss;
  oss << uuid;
  EXPECT_EQ("01020304-0506-0708-090a-0b0c0d0e0f10", oss.str());
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
