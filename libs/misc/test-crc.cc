//==========================================================================
// ObTools::Misc: test-crc.cc
//
// GTest harness for CRC-16 functions
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-misc.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;
using namespace ObTools::Misc;

TEST(CRCTest, TestCRC16Reflected)
{
  CRC crc(CRC::ALGORITHM_CRC16, true, false);
  auto result = crc.calculate("123456789");
  EXPECT_NE(0, result);
}

TEST(CRCTest, TestCRC16NonReflected)
{
  CRC crc(CRC::ALGORITHM_CRC16, false, false);
  auto result = crc.calculate("123456789");
  EXPECT_NE(0, result);
}

TEST(CRCTest, TestCCITTReflected)
{
  CRC crc(CRC::ALGORITHM_CCITT, true, false);
  auto result = crc.calculate("123456789");
  EXPECT_NE(0, result);
}

TEST(CRCTest, TestCCITTNonReflected)
{
  CRC crc(CRC::ALGORITHM_CCITT, false, false);
  auto result = crc.calculate("123456789");
  EXPECT_NE(0, result);
}

TEST(CRCTest, TestCCITTZero)
{
  CRC crc(CRC::ALGORITHM_CCITT_ZERO, false, false);
  auto result = crc.calculate("123456789");
  EXPECT_NE(0, result);
}

TEST(CRCTest, TestCCITTMod)
{
  CRC crc(CRC::ALGORITHM_CCITT_MOD, false, false);
  auto result = crc.calculate("123456789");
  EXPECT_NE(0, result);
}

TEST(CRCTest, TestCCITTModReflected)
{
  CRC crc(CRC::ALGORITHM_CCITT_MOD, true, false);
  auto result = crc.calculate("123456789");
  EXPECT_NE(0, result);
}

TEST(CRCTest, TestFlip)
{
  CRC crc_noflip(CRC::ALGORITHM_CRC16, true, false);
  CRC crc_flip(CRC::ALGORITHM_CRC16, true, true);
  auto r1 = crc_noflip.calculate("test");
  auto r2 = crc_flip.calculate("test");
  EXPECT_EQ(r1 ^ 0xFFFF, r2);
}

TEST(CRCTest, TestRawDataInterface)
{
  CRC crc(CRC::ALGORITHM_CRC16, true, false);
  string data = "hello";
  auto r1 = crc.calculate(data);
  auto r2 = crc.calculate(reinterpret_cast<const unsigned char *>(data.c_str()),
                           data.size());
  EXPECT_EQ(r1, r2);
}

TEST(CRCTest, TestDifferentDataProducesDifferentCRC)
{
  CRC crc(CRC::ALGORITHM_CCITT, false, false);
  auto r1 = crc.calculate("hello");
  auto r2 = crc.calculate("world");
  EXPECT_NE(r1, r2);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
