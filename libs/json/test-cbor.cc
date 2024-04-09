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

TEST(CBOR, TestBooleanOutput)
{
  EXPECT_EQ("f4", Text::btox(Value(Value::FALSE_).cbor()));
  EXPECT_EQ("f5", Text::btox(Value(Value::TRUE_).cbor()));
}

TEST(CBOR, TestNullUndefinedOutput)
{
  EXPECT_EQ("f6", Text::btox(Value(Value::NULL_).cbor()));
  EXPECT_EQ("f7", Text::btox(Value().cbor()));
}

TEST(CBOR, TestBinaryOutput)
{
  vector<unsigned char> b{42, 99};
  EXPECT_EQ("422a63", Text::btox(Value(b).cbor()));

  vector<unsigned char> b24;
  for(int i=0; i<24; i++) b24.push_back(i);
  EXPECT_EQ("5818000102030405060708090a0b0c0d0e0f1011121314151617",
            Text::btox(Value(b24).cbor()));
}

TEST(CBOR, TestStringOutput)
{
  EXPECT_EQ("60", Text::btox(Value("").cbor()));
  EXPECT_EQ("6161", Text::btox(Value("a").cbor()));
  EXPECT_EQ("6449455446", Text::btox(Value("IETF").cbor()));
  EXPECT_EQ("62225c", Text::btox(Value("\"\\").cbor()));
  EXPECT_EQ("62c3bc", Text::btox(Value("\u00fc").cbor()));
  EXPECT_EQ("63e6b0b4", Text::btox(Value("\u6c34").cbor()));

  // Trigger 1 byte length (24)
  EXPECT_EQ("7818313233343536373839303132333435363738393031323334",
            Text::btox(Value("123456789012345678901234").cbor()));
}

TEST(CBOR, TestArrayOutput)
{
  Value a(Value::ARRAY);
  a.add(Value(42));
  a.add(Value(Value::TRUE_));
  a.add(Value("foo"));

  EXPECT_EQ("83182af563666f6f", Text::btox(a.cbor()));
  EXPECT_EQ("80", Text::btox(Value(Value::ARRAY).cbor()));
}

TEST(CBOR, TestNestedArrayOutput)
{
  Value a(Value::ARRAY);
  a.add(1);

  auto& a2 = a.add(Value::ARRAY);
  a2.add(2);
  a2.add(3);

  auto& a3 = a.add(Value::ARRAY);
  a3.add(4);
  a3.add(5);

  EXPECT_EQ("8301820203820405", Text::btox(a.cbor()));
}

TEST(CBOR, TestObjectOutput)
{
  Value o(Value::OBJECT);
  o.put("a", Value(42));
  o.put("b", Value(Value::TRUE_));
  o.put("c", Value("foo"));

  EXPECT_EQ("a36161182a6162f5616363666f6f", Text::btox(o.cbor()));
}

TEST(CBOR, TestNestedObjectOutput)
{
  Value o(Value::OBJECT);
  o.put("a", Value(42));

  auto& o2 = o.put("s", Value::OBJECT);
  o2.put("b", Value(Value::TRUE_));
  o2.put("c", Value("foo"));

  EXPECT_EQ("a26161182a6173a26162f5616363666f6f", Text::btox(o.cbor()));
}

TEST(CBOR, TestArrayNestedInObjectOutput)
{
  Value o(Value::OBJECT);
  o.put("a", Value(42));

  auto& a = o.put("A", Value::ARRAY);
  a.add(Value(Value::TRUE_));
  a.add(Value("foo"));

  // Note A comes before a
  EXPECT_EQ("a2614182f563666f6f6161182a", Text::btox(o.cbor()));
}

TEST(CBOR, TestObjectNestedInArrayOutput)
{
  Value a(Value::ARRAY);
  a.add("a");

  auto& o = a.add(Value::OBJECT);
  o.put("b", Value("c"));

  EXPECT_EQ("826161a161626163", Text::btox(a.cbor()));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
