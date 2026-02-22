//==========================================================================
// ObTools::Misc: test-md5.cc
//
// GTest harness for MD5 functions
// Test vectors from RFC 1321
//
// Copyright (c) 2026 Paul Clark.
//==========================================================================

#include "ot-misc.h"
#include <gtest/gtest.h>

namespace {

using namespace std;
using namespace ObTools;
using namespace ObTools::Misc;

TEST(MD5Test, TestEmptyString)
{
  MD5 md5;
  EXPECT_EQ("d41d8cd98f00b204e9800998ecf8427e", md5.sum(""));
}

TEST(MD5Test, TestA)
{
  MD5 md5;
  EXPECT_EQ("0cc175b9c0f1b6a831c399e269772661", md5.sum("a"));
}

TEST(MD5Test, TestABC)
{
  MD5 md5;
  EXPECT_EQ("900150983cd24fb0d6963f7d28e17f72", md5.sum("abc"));
}

TEST(MD5Test, TestMessageDigest)
{
  MD5 md5;
  EXPECT_EQ("f96b697d7cb7938d525a2f31aaf161d0",
            md5.sum("message digest"));
}

TEST(MD5Test, TestAlphabet)
{
  MD5 md5;
  EXPECT_EQ("c3fcd3d76192e4007dfb496cca67e13b",
            md5.sum("abcdefghijklmnopqrstuvwxyz"));
}

TEST(MD5Test, TestLong)
{
  MD5 md5;
  // Test with data longer than one 64-byte block
  EXPECT_EQ("d174ab98d277d9f5a5611c2c9f419d9f",
            md5.sum("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
                    "0123456789"));
}

TEST(MD5Test, TestVeryLong)
{
  MD5 md5;
  // Multiple full blocks
  string input;
  for (int i = 0; i < 8; i++) input += "1234567890";
  EXPECT_EQ("57edf4a22be3c955ac49da2e2107b67a", md5.sum(input));
}

TEST(MD5Test, TestSumBase64)
{
  MD5 md5;
  string result = md5.sum_base64("");
  EXPECT_FALSE(result.empty());
  // d41d8cd98f00b204e9800998ecf8427e in base64
  EXPECT_EQ("1B2M2Y8AsgTpgAmY7PhCfg==", result);
}

TEST(MD5Test, TestHashToInt)
{
  MD5 md5;
  uint64_t h1 = md5.hash_to_int("hello");
  uint64_t h2 = md5.hash_to_int("world");
  EXPECT_NE(h1, h2);
  // Same input should give same result
  EXPECT_EQ(h1, md5.hash_to_int("hello"));
}

TEST(MD5Test, TestReuseAfterSum)
{
  MD5 md5;
  string r1 = md5.sum("hello");
  string r2 = md5.sum("world");
  EXPECT_NE(r1, r2);
  // Verify reuse gives same result
  string r3 = md5.sum("hello");
  EXPECT_EQ(r1, r3);
}

TEST(MD5Test, TestRawDigest)
{
  MD5 md5;
  unsigned char digest[16];
  md5.sum("", digest);
  // First byte of MD5("") = 0xd4
  EXPECT_EQ(0xd4, digest[0]);
}

TEST(MD5Test, TestIncrementalUpdate)
{
  // Compare incremental update with single-shot
  MD5 md5_single;
  string r1 = md5_single.sum("helloworld");

  MD5 md5_inc;
  md5_inc.update("hello", 5);
  md5_inc.update("world", 5);
  unsigned char digest[16];
  md5_inc.finalise(digest);

  ostringstream oss;
  for (int i = 0; i < 16; i++)
    oss << hex << setfill('0') << setw(2) << static_cast<unsigned>(digest[i]);
  EXPECT_EQ(r1, oss.str());
}

TEST(MD5Test, TestIncrementalUpdateCrossingBlockBoundary)
{
  // First update fills partial block (60 bytes), second update crosses
  // the 64-byte boundary, triggering the buffer-crossing code path
  // (md5.cc lines 96-100)
  string data(70, 'A');
  MD5 md5_single;
  string r1 = md5_single.sum(data);

  MD5 md5_inc;
  md5_inc.update(data.c_str(), 60);
  md5_inc.update(data.c_str() + 60, 10);
  unsigned char digest[16];
  md5_inc.finalise(digest);

  ostringstream oss;
  for (int i = 0; i < 16; i++)
    oss << hex << setfill('0') << setw(2) << static_cast<unsigned>(digest[i]);
  EXPECT_EQ(r1, oss.str());
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
