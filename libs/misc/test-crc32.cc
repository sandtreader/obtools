//==========================================================================
// ObTools::Misc: test-crc32.cc
//
// GTest harness for CRC-32 functions
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-misc.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;
using namespace ObTools::Misc;

// Known CRC32 test vector: "123456789" -> 0xCBF43926 (standard reflected+flip)
TEST(CRC32Test, TestKnownVector)
{
  CRC32 crc;  // default: CRC32, reflected=true, flip=true
  auto result = crc.calculate("123456789");
  EXPECT_EQ(0xCBF43926u, result);
}

TEST(CRC32Test, TestCRC32CReflected)
{
  CRC32 crc(CRC32::ALGORITHM_CRC32C, true, true);
  auto result = crc.calculate("123456789");
  // CRC-32C of "123456789" = 0xE3069283
  EXPECT_EQ(0xE3069283u, result);
}

TEST(CRC32Test, TestNonReflected)
{
  CRC32 crc(CRC32::ALGORITHM_CRC32, false, true);
  auto result = crc.calculate("123456789");
  EXPECT_NE(0u, result);
}

TEST(CRC32Test, TestNoFlip)
{
  CRC32 crc_noflip(CRC32::ALGORITHM_CRC32, true, false);
  CRC32 crc_flip(CRC32::ALGORITHM_CRC32, true, true);
  auto r1 = crc_noflip.calculate("test");
  auto r2 = crc_flip.calculate("test");
  EXPECT_EQ(r1 ^ 0xFFFFFFFF, r2);
}

TEST(CRC32Test, TestStreamAPI)
{
  CRC32 crc;
  auto init = crc.initialiser();
  auto mid = crc.consume(reinterpret_cast<const unsigned char *>("12345"), 5,
                         init);
  mid = crc.consume(reinterpret_cast<const unsigned char *>("6789"), 4, mid);
  auto result = crc.finalise(mid);
  EXPECT_EQ(0xCBF43926u, result);
}

TEST(CRC32Test, TestStreamMatchesSingleShot)
{
  CRC32 crc;
  auto single = crc.calculate("hello world");

  auto init = crc.initialiser();
  auto mid = crc.consume(
    reinterpret_cast<const unsigned char *>("hello "), 6, init);
  mid = crc.consume(
    reinterpret_cast<const unsigned char *>("world"), 5, mid);
  auto stream = crc.finalise(mid);

  EXPECT_EQ(single, stream);
}

TEST(CRC32Test, TestRawDataInterface)
{
  CRC32 crc;
  string data = "test data";
  auto r1 = crc.calculate(data);
  auto r2 = crc.calculate(
    reinterpret_cast<const unsigned char *>(data.c_str()), data.size());
  EXPECT_EQ(r1, r2);
}

TEST(CRC32Test, TestDifferentDataProducesDifferentCRC)
{
  CRC32 crc;
  EXPECT_NE(crc.calculate("hello"), crc.calculate("world"));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
