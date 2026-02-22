//==========================================================================
// ObTools::Misc: test-random.cc
//
// GTest harness for Random number generator
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-misc.h"
#include <gtest/gtest.h>
#include <set>

namespace {

using namespace std;
using namespace ObTools;
using namespace ObTools::Misc;

TEST(RandomTest, TestGenerateBinaryReturnsRequestedLength)
{
  Random rng;
  unsigned char buf[32];
  rng.generate_binary(buf, 32);
  // Can't really test content is random, but should not crash
}

TEST(RandomTest, TestGenerateBinaryVector)
{
  Random rng;
  auto buf = rng.generate_binary(16);
  EXPECT_EQ(16, buf.size());
}

TEST(RandomTest, TestGenerateHex)
{
  Random rng;
  string hex = rng.generate_hex(8);
  EXPECT_EQ(16, hex.size());  // 8 bytes = 16 hex chars
  // All characters should be valid hex
  for (auto c : hex)
    EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
}

TEST(RandomTest, TestGenerate32)
{
  Random rng;
  set<uint32_t> values;
  for (int i = 0; i < 100; i++)
    values.insert(rng.generate_32());
  // Should have many distinct values
  EXPECT_GT(values.size(), 90u);
}

TEST(RandomTest, TestGenerate64)
{
  Random rng;
  set<uint64_t> values;
  for (int i = 0; i < 100; i++)
    values.insert(rng.generate_64());
  EXPECT_GT(values.size(), 90u);
}

TEST(RandomTest, TestGenerateUpTo)
{
  Random rng;
  for (int i = 0; i < 1000; i++)
  {
    auto v = rng.generate_up_to(10);
    EXPECT_LT(v, 10u);
  }
}

TEST(RandomTest, TestReseedingDoesNotCrash)
{
  // Random reseeds every 67 calls; call enough times to trigger
  Random rng;
  for (int i = 0; i < 200; i++)
    rng.generate_32();
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
