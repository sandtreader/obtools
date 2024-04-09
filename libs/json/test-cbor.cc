//==========================================================================
// ObTools::JSON: test-cbor.cc
//
// Test harness for JSON CBOR output etc.
// Test cases from RFC8949 Appendix A plus some edge cases

// Copyright (c) 2024 Paul Clark.
//==========================================================================

#include <gtest/gtest.h>
#include "ot-json.h"
#include <limits.h>

using namespace std;
using namespace ObTools;
using namespace ObTools::JSON;

TEST(CBOR, TestTinyPositiveIntegerOutput)
{
  EXPECT_EQ("00", Text::btox(Value(0).cbor()));
  EXPECT_EQ("01", Text::btox(Value(1).cbor()));
  EXPECT_EQ("0a", Text::btox(Value(10).cbor()));
  EXPECT_EQ("17", Text::btox(Value(23).cbor()));
}

TEST(CBOR, Test1BytePositiveIntegerOutput)
{
  EXPECT_EQ("1818", Text::btox(Value(24).cbor()));
  EXPECT_EQ("1819", Text::btox(Value(25).cbor()));
  EXPECT_EQ("1864", Text::btox(Value(100).cbor()));
  EXPECT_EQ("18ff", Text::btox(Value(255).cbor()));
}

TEST(CBOR, Test2BytePositiveIntegerOutput)
{
  EXPECT_EQ("190100", Text::btox(Value(256).cbor()));
  EXPECT_EQ("1903e8", Text::btox(Value(1000).cbor()));
  EXPECT_EQ("19ffff", Text::btox(Value(65535).cbor()));
}

TEST(CBOR, Test4BytePositiveIntegerOutput)
{
  EXPECT_EQ("1a00010000", Text::btox(Value(65536).cbor()));
  EXPECT_EQ("1a000f4240", Text::btox(Value(1000000).cbor()));
  EXPECT_EQ("1affffffff", Text::btox(Value(0xffffffffUL).cbor()));
}

TEST(CBOR, Test8BytePositiveIntegerOutput)
{
  EXPECT_EQ("1b0000000100000000", Text::btox(Value(0x100000000UL).cbor()));
  EXPECT_EQ("1b000000e8d4a51000", Text::btox(Value(1000000000000).cbor()));

  // Note we can't represent 2^64-1 because Value.n is signed
  EXPECT_EQ("1b7fffffffffffffff",
            Text::btox(Value((uint64_t)LLONG_MAX).cbor()));
}

TEST(CBOR, TestTinyNegativeIntegerOutput)
{
  EXPECT_EQ("20", Text::btox(Value(-1).cbor()));
  EXPECT_EQ("29", Text::btox(Value(-10).cbor()));
  EXPECT_EQ("36", Text::btox(Value(-23).cbor()));
  EXPECT_EQ("37", Text::btox(Value(-24).cbor()));
}

TEST(CBOR, Test1ByteNegativeIntegerOutput)
{
  EXPECT_EQ("3863", Text::btox(Value(-100).cbor()));
  EXPECT_EQ("38ff", Text::btox(Value(-256).cbor()));
}

TEST(CBOR, Test2ByteNegativeIntegerOutput)
{
  EXPECT_EQ("390100", Text::btox(Value(-257).cbor()));
  EXPECT_EQ("3903e7", Text::btox(Value(-1000).cbor()));
  EXPECT_EQ("39ffff", Text::btox(Value(-65536).cbor()));
}

TEST(CBOR, Test4ByteNegativeIntegerOutput)
{
  EXPECT_EQ("3a00010000", Text::btox(Value(-65537).cbor()));
  EXPECT_EQ("3affffffff", Text::btox(Value(-4294967296).cbor()));
}

TEST(CBOR, Test8ByteNegativeIntegerOutput)
{
  EXPECT_EQ("3b0000000100000000", Text::btox(Value(-4294967297).cbor()));
  EXPECT_EQ("3b7fffffffffffffff", Text::btox(Value((int64_t)LLONG_MIN).cbor()));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
