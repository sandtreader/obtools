//==========================================================================
// ObTools::Text: test-base64.cc
//
// GTest harness for text library Base64 functions
// Test cases from Base64.guru
//
// Copyright (c) 2021 Paul Clark.
//==========================================================================

#include "ot-text.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;

TEST(Base64Test, TestBasicEncode)
{
  Text::Base64 base64;
  EXPECT_EQ("PDw/Pz8+Pg==", base64.encode("<<??\?>>"));  // Escape trigraphs!?
}

TEST(Base64Test, TestBinaryEncode)
{
  Text::Base64 base64;
  vector<byte> binary{ byte{42}, byte{99} };
  EXPECT_EQ("KmM=", base64.encode(binary));
}

TEST(Base64Test, TestBasicDecode)
{
  Text::Base64 base64;
  string text;
  ASSERT_TRUE(base64.decode("PDw/Pz8+Pg==", text));
  EXPECT_EQ("<<??\?>>", text);
}

TEST(Base64Test, TestVectorDecode)
{
  Text::Base64 base64;
  vector<byte> binary;
  ASSERT_TRUE(base64.decode("KmM=", binary));
  ASSERT_EQ(2, binary.size());
  EXPECT_EQ(byte(42), binary[0]);
  EXPECT_EQ(byte(99), binary[1]);
}

TEST(Base64URLTest, TestBasicEncode)
{
  Text::Base64URL base64;
  EXPECT_EQ("PDw_Pz8-Pg", base64.encode("<<??\?>>"));
}

TEST(Base64URLTest, TestBasicDecode)
{
  Text::Base64URL base64;
  string text;
  ASSERT_TRUE(base64.decode("PDw_Pz8-Pg", text));
  EXPECT_EQ("<<??\?>>", text);
}

TEST(Base64Test, TestEncodeUint64)
{
  Text::Base64 base64;
  // Small value fits in 4 bytes
  string encoded = base64.encode(uint64_t{0x01020304});
  uint64_t decoded;
  ASSERT_TRUE(base64.decode(encoded, decoded));
  EXPECT_EQ(0x01020304ULL, decoded);
}

TEST(Base64Test, TestEncodeUint64Large)
{
  Text::Base64 base64;
  // Large value needs 8 bytes
  string encoded = base64.encode(uint64_t{0x0102030405060708ULL});
  uint64_t decoded;
  ASSERT_TRUE(base64.decode(encoded, decoded));
  EXPECT_EQ(0x0102030405060708ULL, decoded);
}

TEST(Base64Test, TestDecodeIgnoresUnknownChars)
{
  Text::Base64 base64;
  // Whitespace and line breaks should be ignored
  string text;
  ASSERT_TRUE(base64.decode("Km\r\nM=", text));
  EXPECT_EQ("\x2a\x63", text);
}

TEST(Base64Test, TestDecodeOverflow)
{
  Text::Base64 base64;
  unsigned char buf[2];
  // "KmM=" decodes to 2 bytes; buffer only holds 2, so fits
  auto len = base64.decode("KmM=", buf, 2);
  EXPECT_EQ(2, len);

  // Overflow in the main loop: "QUJD" decodes to "ABC" (3 bytes)
  // With a 1-byte buffer, the inner loop hits the overflow return on line 178
  unsigned char tiny[1];
  auto olen = base64.decode("QUJD", tiny, 1);
  EXPECT_EQ(2, olen);  // Returns max_length+1 to indicate overflow

  // Overflow in the remainder section: "QUJDRA" = "ABCD" (4 bytes)
  // First 4 chars ("QUJD") write 3 bytes filling the buffer.
  // Remainder ("RA") attempts to write a 4th byte, hitting line 197
  unsigned char buf3[3];
  auto olen2 = base64.decode("QUJDRA", buf3, 3);
  EXPECT_EQ(4, olen2);  // Returns max_length+1 to indicate overflow
}

TEST(Base64Test, TestDecodeUint64RoundTrip)
{
  Text::Base64 base64;
  uint64_t values[] = {0, 1, 255, 256, 0xFFFFFFFF, 0x100000000ULL,
                       0xFFFFFFFFFFFFFFFFULL};
  for (auto v : values)
  {
    string encoded = base64.encode(v);
    uint64_t decoded;
    ASSERT_TRUE(base64.decode(encoded, decoded));
    EXPECT_EQ(v, decoded);
  }
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
