//==========================================================================
// ObTools::Text: test-uuid.cc
//
// Test harness for text library UUID class
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-text.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;
using namespace ObTools::Text;

TEST(UUIDTest, TestBasicConstruction)
{
  const auto actual = UUID{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                           0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
  const auto expected = array<byte, 16>{{0x01, 0x02, 0x03, 0x04,
                                         0x05, 0x06, 0x07, 0x08,
                                         0x09, 0x0a, 0x0b, 0x0c,
                                         0x0d, 0x0e, 0x0f, 0x10}};
  EXPECT_EQ(expected, actual);
}

TEST(UUIDTest, TestHexStringConstruction)
{
  const auto actual = UUID{"0102030405060708090a0b0c0d0e0f10"};
  const auto expected = array<byte, 16>{{0x01, 0x02, 0x03, 0x04,
                                         0x05, 0x06, 0x07, 0x08,
                                         0x09, 0x0a, 0x0b, 0x0c,
                                         0x0d, 0x0e, 0x0f, 0x10}};
  EXPECT_EQ(expected, actual);
}

TEST(UUIDTest, TestUUIDStringConstruction)
{
  const auto actual = UUID{"01020304-0506-0708-090a-0b0c0d0e0f10"};
  const auto expected = array<byte, 16>{{0x01, 0x02, 0x03, 0x04,
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

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
