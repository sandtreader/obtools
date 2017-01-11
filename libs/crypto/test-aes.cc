//==========================================================================
// ObTools::Crypto: test-aes.cc
//
// Test harness for Crypto library AES encryption/decryption functions
//
// Copyright (c) 2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-crypto.h"

using namespace std;
using namespace ObTools;

TEST(AESTests, TestSetAESCTR)
{
  auto key = Crypto::AESKey{Crypto::AESKey::BITS_128};
  key.set_from_int(42);
  ASSERT_TRUE(key.valid);

  auto iv = Crypto::AESKey{Crypto::AESKey::BITS_128, false};
  iv.set_from_int(958259);
  ASSERT_TRUE(iv.valid);

  auto data = vector<unsigned char>{'H', 'e', 'l', 'l', 'o', ',', ' ',
                                    'w', 'o', 'r', 'l', 'd', '!'};

  auto aes = Crypto::AES{};
  aes.set_key(key);
  aes.set_iv(iv);
  aes.set_ctr(true);
  aes.encrypt(&data[0], data.size());

  const auto expected = vector<unsigned char>{0x41, 0x1f, 0x41, 0x26,
                                              0xe1, 0x7f, 0x46, 0xc7,
                                              0x63, 0xf7, 0x8a, 0x36,
                                              0xd4};
  EXPECT_EQ(expected, data);
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
