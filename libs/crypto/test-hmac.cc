//==========================================================================
// ObTools::Crypto: test-hmac.cc
//
// Test harness for Crypto library HMAC functions
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-crypto.h"
#include "ot-misc.h"
#include <iostream>

using namespace std;
using namespace ObTools;

// Test data:
// echo -n data | openssl dgst -sha1 -hex -hmac key
TEST(HMACTests, TestHMACSHA1)
{
  string digest = Crypto::HMACSHA1::sign("key", "data");
  string digest_hex = Text::btox(digest);
  ASSERT_EQ("104152c5bfdca07bc633eebd46199f0255c9f49d",
            digest_hex);
}

// echo -n data | openssl dgst -sha256 -hex -hmac key
TEST(HMACTests, TestHMACSHA256)
{
  string digest = Crypto::HMACSHA256::sign("key", "data");
  string digest_hex = Text::btox(digest);
  ASSERT_EQ("5031fe3d989c6d1537a013fa6e739da23463fdaec3b70137d828e36ace221bd0",
            digest_hex);
}

// Double HMAC
// echo -n "more data" | openssl dgst -sha256 -hex -mac hmac -macopt hexkey:`echo -n data | openssl dgst -sha256 -hex -hmac key | cut -d " " -f2`
TEST(HMACTests, TestHMACSHA256Twice)
{
  string digest = Crypto::HMACSHA256::sign("key", "data");
  string digest_hex = Text::btox(digest);
  ASSERT_EQ("5031fe3d989c6d1537a013fa6e739da23463fdaec3b70137d828e36ace221bd0",
            digest_hex);
  digest = Crypto::HMACSHA256::sign(digest, "more data");
  digest_hex = Text::btox(digest);
  ASSERT_EQ("9c8a6fa2f19081a10ccd8fb8b4d77bcd66c1e5e37c8299e32ebe1cd47f3ee45a",
            digest_hex);
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
