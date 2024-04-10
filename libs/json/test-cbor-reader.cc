//==========================================================================
// ObTools::JSON: test-cbor-reader.cc
//
// Test harness for JSON CBOR input
// Test cases from RFC8949 Appendix A plus some edge cases
//
// Copyright (c) 2024 Paul Clark.
//==========================================================================

#include <gtest/gtest.h>
#include "ot-json.h"
#include <limits.h>

using namespace std;
using namespace ObTools;
using namespace ObTools::JSON;

// Get JSON string from CBOR hex
string decode(const string& hex)
{
  auto binary = Text::xtob(hex);
  Channel::StringReader sr(binary);
  CBORReader cr(sr);
  return cr.decode().str();
}

TEST(CBORReader, TestTinyPositiveInteger)
{
  EXPECT_EQ("0",  decode("00"));
  EXPECT_EQ("1",  decode("01"));
  EXPECT_EQ("10", decode("0a"));
  EXPECT_EQ("23", decode("17"));
}

TEST(CBORReader, Test1BytePositiveInteger)
{
  EXPECT_EQ("24",  decode("1818"));
  EXPECT_EQ("25",  decode("1819"));
  EXPECT_EQ("100", decode("1864"));
  EXPECT_EQ("255", decode("18ff"));
}

TEST(CBORReader, Test2BytePositiveInteger)
{
  EXPECT_EQ("256",   decode("190100"));
  EXPECT_EQ("1000",  decode("1903e8"));
  EXPECT_EQ("65535", decode("19ffff"));
}

TEST(CBORReader, Test4BytePositiveInteger)
{
  EXPECT_EQ("65536",      decode("1a00010000"));
  EXPECT_EQ("1000000",    decode("1a000f4240"));
  EXPECT_EQ("4294967295", decode("1affffffff"));
}

TEST(CBORReader, Test8BytePositiveInteger)
{
  EXPECT_EQ("4294967296",          decode("1b0000000100000000"));
  EXPECT_EQ("1000000000000",       decode("1b000000e8d4a51000"));
  EXPECT_EQ("9223372036854775807", decode("1b7fffffffffffffff"));
}

TEST(CBORReader, TestTinyNegativeInteger)
{
  EXPECT_EQ("-1",  decode("20"));
  EXPECT_EQ("-10", decode("29"));
  EXPECT_EQ("-23", decode("36"));
  EXPECT_EQ("-24", decode("37"));
}

TEST(CBORReader, Test1ByteNegativeInteger)
{
  EXPECT_EQ("-100", decode("3863"));
  EXPECT_EQ("-256", decode("38ff"));
}

TEST(CBORReader, Test2ByteNegativeInteger)
{
  EXPECT_EQ("-257",   decode("390100"));
  EXPECT_EQ("-1000",  decode("3903e7"));
  EXPECT_EQ("-65536", decode("39ffff"));
}

TEST(CBORReader, Test4ByteNegativeInteger)
{
  EXPECT_EQ("-65537",      decode("3a00010000"));
  EXPECT_EQ("-4294967296", decode("3affffffff"));
}

TEST(CBORReader, Test8ByteNegativeInteger)
{
  EXPECT_EQ("-4294967297",          decode("3b0000000100000000"));
  EXPECT_EQ("-9223372036854775808", decode("3b7fffffffffffffff"));
}

TEST(CBORReader, TestBoolean)
{
  EXPECT_EQ("false", decode("f4"));
  EXPECT_EQ("true",  decode("f5"));
}

TEST(CBORReader, TestNullUndefined)
{
  EXPECT_EQ("null",      decode("f6"));
  EXPECT_EQ("undefined", decode("f7"));
}

TEST(CBORReader, TestBinary)
{
  EXPECT_EQ("\"KmM=\"", decode("422a63"));
}

TEST(CBORReader, TestString)
{
  EXPECT_EQ("\"\"", decode("60"));
  EXPECT_EQ("\"a\"", decode("6161"));
  EXPECT_EQ("\"IETF\"", decode("6449455446"));

  // 1 byte length
  EXPECT_EQ("\"123456789012345678901234\"",
            decode("7818313233343536373839303132333435363738393031323334"));
}

TEST(CBORReader, TestArray)
{
  EXPECT_EQ("[]", decode("80"));
  EXPECT_EQ("[42,true,\"foo\"]", decode("83182af563666f6f"));
  EXPECT_EQ("[1,[2,3],[4,5]]", decode("8301820203820405"));
}

TEST(CBORReader, TestObject)
{
  EXPECT_EQ("{}", decode("a0"));
  EXPECT_EQ("{\"a\":42,\"b\":true,\"c\":\"foo\"}",
            decode("a36161182a6162f5616363666f6f"));
  EXPECT_EQ("{\"a\":42,\"s\":{\"b\":true,\"c\":\"foo\"}}",
            decode("a26161182a6173a26162f5616363666f6f"));
}

TEST(CBORReader, TestNestedThings)
{
  EXPECT_EQ("{\"A\":[true,\"foo\"],\"a\":42}",   // Note A < a
            decode("a2614182f563666f6f6161182a"));
  EXPECT_EQ("[\"a\",{\"b\":\"c\"}]",
            decode("826161a161626163"));
}

TEST(CBORReader, TestIndefiniteArrayReadInOneHit)
{
  EXPECT_EQ("[]", decode("9fff"));
  EXPECT_EQ("[42,true,\"foo\"]", decode("9f182af563666f6fff"));
  EXPECT_EQ("[1,[2,3],[4,5]]", decode("9f019f0203ff820405ff"));
}

TEST(CBORReader, TestIndefiniteObjectReadInOneHit)
{
  EXPECT_EQ("{}", decode("bfff"));
  EXPECT_EQ("{\"a\":42,\"b\":true,\"c\":\"foo\"}",
            decode("bf6161182a6162f5616363666f6fff"));
  EXPECT_EQ("{\"a\":42,\"s\":{\"b\":true,\"c\":\"foo\"}}",
            decode("bf6161182a6173bf6162f5616363666f6fffff"));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
