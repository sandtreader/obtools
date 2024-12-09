//==========================================================================
// ObTools::Crypto: test-keypair.cc
//
// Test harness for Crypto library Key Pair functions
//
// Copyright (c) 2024 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-crypto.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::Crypto;

#if OPENSSL_VERSION_MAJOR >= 3
TEST(KeyPairECTests, TestFailsToVerifyIncorrectSECP256K1Signature)
{
  const auto key_data = vector<byte>{
    byte{0x04}, byte{0x22}, byte{0x66}, byte{0xea}, byte{0x97}, byte{0x3a},
    byte{0x98}, byte{0x7b}, byte{0x85}, byte{0x69}, byte{0x4a}, byte{0x70},
    byte{0x0b}, byte{0x17}, byte{0x73}, byte{0x10}, byte{0x9f}, byte{0xba},
    byte{0x67}, byte{0x4a}, byte{0x03}, byte{0xfd}, byte{0xa7}, byte{0x1a},
    byte{0x3e}, byte{0xf8}, byte{0x46}, byte{0x44}, byte{0x0d}, byte{0xcf},
    byte{0xf3}, byte{0x09}, byte{0x5d}, byte{0x49}, byte{0x65}, byte{0x7f},
    byte{0x7a}, byte{0xd0}, byte{0x4b}, byte{0xb5}, byte{0x1c}, byte{0x7e},
    byte{0x48}, byte{0x91}, byte{0x6b}, byte{0xcb}, byte{0x17}, byte{0x56},
    byte{0xcb}, byte{0xb9}, byte{0x02}, byte{0x73}, byte{0x0a}, byte{0x04},
    byte{0xe1}, byte{0x60}, byte{0x56}, byte{0xa6}, byte{0xc5}, byte{0xe3},
    byte{0x41}, byte{0x2a}, byte{0x36}, byte{0x30}, byte{0xcf},
  };
  const auto key = KeyPair::create_secp256k1_pub(key_data);
  const auto message = vector<byte>{
    byte{'H'}, byte{'e'}, byte{'l'}, byte{'l'}, byte{'o'}, byte{','},
    byte{' '}, byte{'w'}, byte{'o'}, byte{'r'}, byte{'l'}, byte{'d'},
    byte{'!'},
  };
  const auto signature = vector<byte>{
    byte{0x30}, byte{0x46}, byte{0x02}, byte{0x21}, byte{0x00}, byte{0xa3},
    byte{0xde}, byte{0xad}, byte{0x3c}, byte{0x41}, byte{0x2f}, byte{0x19},
    byte{0x5f}, byte{0x50}, byte{0x13}, byte{0x27}, byte{0x81}, byte{0x2f},
    byte{0x29}, byte{0xd4}, byte{0x4a}, byte{0xcd}, byte{0x5b}, byte{0x7f},
    byte{0x08}, byte{0x9e}, byte{0x68}, byte{0x39}, byte{0x3a}, byte{0xa6},
    byte{0x5a}, byte{0x4c}, byte{0x60}, byte{0xb0}, byte{0xbf}, byte{0xd9},
    byte{0xb1}, byte{0x02}, byte{0x21}, byte{0x00}, byte{0xa0}, byte{0xf6},
    byte{0xf4}, byte{0xca}, byte{0x66}, byte{0x16}, byte{0x9f}, byte{0x0a},
    byte{0x27}, byte{0xdb}, byte{0x27}, byte{0xc2}, byte{0xeb}, byte{0xaf},
    byte{0x36}, byte{0xff}, byte{0xaa}, byte{0xd8}, byte{0xd5}, byte{0x94},
    byte{0x02}, byte{0xb9}, byte{0x13}, byte{0xfa}, byte{0xb2}, byte{0x75},
    byte{0xd3}, byte{0x07}, byte{0xa7}, byte{0xe5}, byte{0x59}, byte{0x25},
  };
  EXPECT_FALSE(key->verify(message, signature));
}

TEST(KeyPairECTests, TestVerfiesCorrectSECP256K1Signature)
{
  const auto key_data = vector<byte>{
    byte{0x40}, byte{0x33}, byte{0x5b}, byte{0xf1}, byte{0x16}, byte{0x71},
    byte{0x39}, byte{0x1b}, byte{0x1b}, byte{0xea}, byte{0xd9}, byte{0x91},
    byte{0x14}, byte{0xdf}, byte{0xc3}, byte{0x0a}, byte{0x5b}, byte{0x28},
    byte{0x8b}, byte{0x99}, byte{0x42}, byte{0xe3}, byte{0x1a}, byte{0x48},
    byte{0x51}, byte{0xaa}, byte{0xaf}, byte{0x8e}, byte{0x0e}, byte{0x87},
    byte{0x87}, byte{0x51},
  };
  const auto key = KeyPair::create_secp256k1(key_data);
  const auto message = vector<byte>{
    byte{'H'}, byte{'e'}, byte{'l'}, byte{'l'}, byte{'o'}, byte{','},
    byte{' '}, byte{'w'}, byte{'o'}, byte{'r'}, byte{'l'}, byte{'d'},
    byte{'!'},
  };
  const auto signature = key->sign(message);
  const auto pub_data = vector<byte>{
    byte{0x04}, byte{0x22}, byte{0x66}, byte{0xea}, byte{0x97}, byte{0x3a},
    byte{0x98}, byte{0x7b}, byte{0x85}, byte{0x69}, byte{0x4a}, byte{0x70},
    byte{0x0b}, byte{0x17}, byte{0x73}, byte{0x10}, byte{0x9f}, byte{0xba},
    byte{0x67}, byte{0x4a}, byte{0x03}, byte{0xfd}, byte{0xa7}, byte{0x1a},
    byte{0x3e}, byte{0xf8}, byte{0x46}, byte{0x44}, byte{0x0d}, byte{0xcf},
    byte{0xf3}, byte{0x09}, byte{0x5d}, byte{0x49}, byte{0x65}, byte{0x7f},
    byte{0x7a}, byte{0xd0}, byte{0x4b}, byte{0xb5}, byte{0x1c}, byte{0x7e},
    byte{0x48}, byte{0x91}, byte{0x6b}, byte{0xcb}, byte{0x17}, byte{0x56},
    byte{0xcb}, byte{0xb9}, byte{0x02}, byte{0x73}, byte{0x0a}, byte{0x04},
    byte{0xe1}, byte{0x60}, byte{0x56}, byte{0xa6}, byte{0xc5}, byte{0xe3},
    byte{0x41}, byte{0x2a}, byte{0x36}, byte{0x30}, byte{0xcf},
  };
  const auto pubkey = KeyPair::create_secp256k1_pub(pub_data);
  EXPECT_TRUE(pubkey->verify(message, signature));
}

TEST(KeyPairECTests, TestsSignsMessageCorrectly)
{
}
#endif

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
