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

TEST(CBORReader, TestReadingIndefiniteOpen)
{
  auto binary = Text::xtob("9f00");
  Channel::StringReader sr(binary);
  CBORReader cr(sr);
  ASSERT_TRUE(cr.open_indefinite_array());
  ASSERT_FALSE(cr.open_indefinite_array());
  EXPECT_EQ(2, sr.get_offset());
}

TEST(CBORReader, TestReadingIndefiniteArrayPieceWise)
{
  auto binary = Text::xtob("9f019f0203ff820405ff");
  Channel::StringReader sr(binary);
  CBORReader cr(sr);

  // Note this basically replicates what the reader does if you just
  // do it in one hit without calling open_indefinite_array first, but
  // of course you are in control
  ASSERT_TRUE(cr.open_indefinite_array());

  Value array(Value::ARRAY);
  for(;;)
  {
    auto v = cr.decode();
    if (v.type == Value::BREAK) break;
    array.a.push_back(v);
  }

  EXPECT_EQ("[1,[2,3],[4,5]]", array.str());
}

TEST(CBORReader, TestReadTagByteString)
{
  EXPECT_EQ("\"WCGDWBzTqRK7xp0Y+fh1TE95rcuu03OqZwNETUZf0xBHoAA=\"",
            decode("d818582183581cd3a912bbc69d18f9f8754c4f79adcbaed373aa6703444d465fd31047a000"));
}

TEST(CBORReader, TestDefiniteMapWithIntegerKey)
{
  // Map {42: "hello"} — key is positive integer 42 (0x182a),
  // value is string "hello" (0x6568656c6c6f)
  // a1 = map of 1 item, 182a = uint 42, 6568656c6c6f = "hello"
  auto result = decode("a1182a6568656c6c6f");
  EXPECT_EQ("{\"42\":\"hello\"}", result);
}

TEST(CBORReader, TestIndefiniteMapWithIntegerKey)
{
  // Indefinite map {42: "hello"} — bf...ff
  auto result = decode("bf182a6568656c6c6fff");
  EXPECT_EQ("{\"42\":\"hello\"}", result);
}

TEST(CBORReader, TestDefiniteMapWithBooleanKeyThrows)
{
  // Map with boolean key (f4 = false) — not string or integer
  // a1 = map of 1, f4 = false, 01 = value 1
  auto binary = Text::xtob("a1f401");
  Channel::StringReader sr(binary);
  CBORReader cr(sr);
  EXPECT_THROW(cr.decode(), Channel::Error);
}

TEST(CBORReader, TestIndefiniteMapWithBooleanKeyThrows)
{
  // Indefinite map with boolean key
  auto binary = Text::xtob("bff401ff");
  Channel::StringReader sr(binary);
  CBORReader cr(sr);
  EXPECT_THROW(cr.decode(), Channel::Error);
}

TEST(CBORReader, TestUnhandledSemanticTagThrows)
{
  // Tag 32 — d8 20, then a string "hello"
  auto binary = Text::xtob("d8206568656c6c6f");
  Channel::StringReader sr(binary);
  CBORReader cr(sr);
  EXPECT_THROW(cr.decode(), Channel::Error);
}

TEST(CBORReader, TestUnhandledFloatSimpleThrows)
{
  // Simple value 16 (0xf0 = major 7, additional info 16)
  auto binary = Text::xtob("f0");
  Channel::StringReader sr(binary);
  CBORReader cr(sr);
  EXPECT_THROW(cr.decode(), Channel::Error);
}

TEST(CBORReader, TestReservedAdditionalInfoThrows)
{
  // Additional info 28 (0x1c) in positive integer: 0x1c
  auto binary = Text::xtob("1c");
  Channel::StringReader sr(binary);
  CBORReader cr(sr);
  EXPECT_THROW(cr.decode(), Channel::Error);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
