//==========================================================================
// ObTools::Text: test-convert.cc
//
// Test harness for text library numeric functions
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-text.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(ConvertTest, TestStringToInt)
{
  ASSERT_EQ(1234567890, Text::stoi("1234567890"));
}

TEST(ConvertTest, TestIntToString)
{
  ASSERT_EQ("1234567890", Text::itos(1234567890));
}

TEST(ConvertTest, TestStringToFloat)
{
  ASSERT_EQ(12345.6789, Text::stof("12345.6789"));
}

TEST(ConvertTest, TestStringToBool)
{
  EXPECT_FALSE(Text::stob(""));
  EXPECT_FALSE(Text::stob("foo"));
  EXPECT_FALSE(Text::stob("no"));
  EXPECT_FALSE(Text::stob("No"));
  EXPECT_FALSE(Text::stob("false"));
  EXPECT_FALSE(Text::stob("FALSE"));
  EXPECT_FALSE(Text::stob("0"));

  EXPECT_TRUE(Text::stob("yes"));
  EXPECT_TRUE(Text::stob("Yes"));
  EXPECT_TRUE(Text::stob("true"));
  EXPECT_TRUE(Text::stob("TRUE"));
  EXPECT_TRUE(Text::stob("1"));
}

  TEST(ConvertTest, TestFloatToString)
{
  ASSERT_EQ("12345.6789", Text::ftos(12345.6789, 0, 4, false));
  ASSERT_EQ("12345.68", Text::ftos(12345.6789, 0, 2, false));
}

TEST(ConvertTest, TestHexStringToInt)
{
  ASSERT_EQ(3735928559, Text::xtoi("deadbeef"));
}

TEST(ConvertTest, TestIntToHexString)
{
  ASSERT_EQ("deadbeef", Text::itox(3735928559));
}

TEST(ConvertTest, TestStringToInt64)
{
  ASSERT_EQ(12345678901234567890ULL, Text::stoi64("12345678901234567890"));
}

TEST(ConvertTest, TestInt64ToString)
{
  ASSERT_EQ("12345678901234567890", Text::i64tos(12345678901234567890ULL));
}

TEST(ConvertTest, TestHexStringToInt64)
{
  ASSERT_EQ(18364758544493064720ULL, Text::xtoi64("fedcba9876543210"));
}

TEST(ConvertTest, TestInt64ToHexString)
{
  ASSERT_EQ("fedcba9876543210", Text::i64tox(18364758544493064720ULL));
}

TEST(ConvertTest, TestStringToFixedPoint)
{
  ASSERT_EQ(-89, Text::stoifix("-0.89", 2));
  ASSERT_EQ(-123456789, Text::stoifix("-12345678900", -2));
}

TEST(ConvertTest, TestFixedPointToString)
{
  ASSERT_EQ("-0.89", Text::ifixtos(-89, 2));
  ASSERT_EQ("-12345678900", Text::ifixtos(-123456789, -2));
}

TEST(ConvertTest, TestHexStringToBinaryBuffer)
{
  unsigned char buf[8];
  unsigned char expected[] = {0xde, 0xad, 0xbe, 0xef, 0x12, 0x34, 0x99, 0x00};
  auto n = Text::xtob("DEADbeef12349900", buf, sizeof(buf));
  ASSERT_EQ(8, n);
  for (unsigned i = 0; i < sizeof(buf); ++i)
    EXPECT_EQ(expected[i], buf[i]);
}

TEST(ConvertTest, TestBadHexReturns0Length)
{
  unsigned char buf[8];
  auto n = Text::xtob("DEADbeefXXX", buf, sizeof(buf));
  ASSERT_EQ(0, n);
}

TEST(ConvertTest, TestHexStringToBinaryString)
{
  unsigned char expected[] = {0xde, 0xad, 0xbe, 0xef, 0x12, 0x34, 0x99, 0x00};
  auto binary = Text::xtob("DEADbeef12349900");
  ASSERT_EQ(8, binary.size());
  for (unsigned i = 0; i < binary.size(); ++i)
    EXPECT_EQ(expected[i], (unsigned char)binary[i]);
}

TEST(ConvertTest, TestBadHexReturnsEmptyString)
{
  auto binary = Text::xtob("DEADbeefXXX");
  ASSERT_EQ(0, binary.size());
}

TEST(ConvertTest, TestHexStringToBinaryVector)
{
  unsigned char expected[] = {0xde, 0xad, 0xbe, 0xef, 0x12, 0x34, 0x99, 0x00};
  vector<byte> binary;
  Text::xtob("DEADbeef12349900", binary);
  ASSERT_EQ(8, binary.size());
  for (unsigned i = 0; i < binary.size(); ++i)
    EXPECT_EQ((byte)expected[i], binary[i]);
}

TEST(ConvertTest, TestHexStringToBinaryVectorStopsAtBadHex)
{
  unsigned char expected[] = {0xde, 0xad, 0xbe, 0xef};
  vector<byte> binary;
  Text::xtob("DEADbeefXXX12349900", binary);
  ASSERT_EQ(4, binary.size());
  for (unsigned i = 0; i < binary.size(); ++i)
    EXPECT_EQ((byte)expected[i], binary[i]);
}

TEST(ConvertTest, TestBinaryToString)
{
  unsigned char buf[4] = {0xde, 0xad, 0xbe, 0xef};
  ASSERT_EQ("deadbeef", Text::btox(buf, sizeof(buf)));
}

TEST(ConvertTest, TestBinaryVectorToString)
{
  vector<byte> buf;
  buf.push_back(byte{0xde});
  buf.push_back(byte{0xad});
  buf.push_back(byte{0xbe});
  buf.push_back(byte{0xef});
  vector<byte> buf2;
  buf2.push_back(byte{0x00});
  buf2.push_back(byte{0x00});
  buf2.push_back(byte{0xbe});
  buf2.push_back(byte{0xef});

  ASSERT_EQ("deadbeef", Text::btox(buf));
  ASSERT_EQ("0000beef", Text::btox(buf2));
}

TEST(ConvertTest, TestBinaryStringToHex)
{
  string data = "\xde\xad\xbe\xef";
  EXPECT_EQ("deadbeef", Text::btox(data));
}

TEST(ConvertTest, TestBinaryUint8VectorToHex)
{
  vector<uint8_t> buf{0xde, 0xad, 0xbe, 0xef};
  EXPECT_EQ("deadbeef", Text::btox(buf));
}

TEST(ConvertTest, TestFloatToStringWithWidth)
{
  EXPECT_EQ("0012345.68", Text::ftos(12345.6789, 10, 2, true));
}

TEST(ConvertTest, TestFixedPointToStringPositive)
{
  EXPECT_EQ("1.23", Text::ifixtos(123, 2));
  EXPECT_EQ("0.01", Text::ifixtos(1, 2));
}

TEST(ConvertTest, TestNegativeIntToString)
{
  EXPECT_EQ("-42", Text::itos(-42));
}

TEST(ConvertTest, TestFixedPointToStringZeroDecimalPlaces)
{
  // decimal_places=0 triggers early return (convert.cc line 73-74)
  EXPECT_EQ("42", Text::ifixtos(42, 0));
  EXPECT_EQ("-7", Text::ifixtos(-7, 0));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
