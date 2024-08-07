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
TEST(KeyPairEdTests, TestFailsToInitialiseInvalidKey)
{
  const auto key_data = vector<byte>{
    byte{0x00}
  };
  const auto key = KeyPair::create_ed25519_pub(key_data);
  EXPECT_FALSE(key->is_valid());
}

TEST(KeyPairEdTests, TestInitialisesValidKey)
{
  const auto key_data = vector<byte>{
    byte{0x6a}, byte{0x1d}, byte{0x51}, byte{0x90},
    byte{0xd5}, byte{0x22}, byte{0xd1}, byte{0x2c},
    byte{0xbd}, byte{0x43}, byte{0xad}, byte{0xf5},
    byte{0xfc}, byte{0x06}, byte{0x5e}, byte{0xdb},
    byte{0x96}, byte{0x2b}, byte{0x76}, byte{0x9e},
    byte{0x09}, byte{0x08}, byte{0xce}, byte{0xb0},
    byte{0xb3}, byte{0x40}, byte{0xf5}, byte{0xf0},
    byte{0xf0}, byte{0x53}, byte{0x92}, byte{0x66},
  };
  const auto key = KeyPair::create_ed25519_pub(key_data);
  EXPECT_TRUE(key->is_valid());
}

TEST(KeyPairEdTests, TestFailsForIncorrectSignature)
{
  const auto key_data = vector<byte>{
    byte{0xb7}, byte{0xa3}, byte{0xc1}, byte{0x2d},
    byte{0xc0}, byte{0xc8}, byte{0xc7}, byte{0x48},
    byte{0xab}, byte{0x07}, byte{0x52}, byte{0x5b},
    byte{0x70}, byte{0x11}, byte{0x22}, byte{0xb8},
    byte{0x8b}, byte{0xd7}, byte{0x8f}, byte{0x60},
    byte{0x0c}, byte{0x76}, byte{0x34}, byte{0x2d},
    byte{0x27}, byte{0xf2}, byte{0x5e}, byte{0x5f},
    byte{0x92}, byte{0x44}, byte{0x4c}, byte{0xde},
  };
  const auto key = KeyPair::create_ed25519_pub(key_data);
  const auto message = vector<byte>{
    byte{'M'}, byte{'e'}, byte{'s'}, byte{'s'},
    byte{'a'}, byte{'g'}, byte{'e'}, byte{' '},
    byte{'f'}, byte{'o'}, byte{'r'}, byte{' '},
    byte{'E'}, byte{'d'}, byte{'2'}, byte{'5'},
    byte{'5'}, byte{'1'}, byte{'9'}, byte{' '},
    byte{'s'}, byte{'i'}, byte{'g'}, byte{'n'},
    byte{'i'}, byte{'n'}, byte{'g'},
  };
  const auto signature = vector<byte>{
    byte{0xde}, byte{0xad}, byte{0xbe}, byte{0xef},
    byte{0x7f}, byte{0xae}, byte{0x4e}, byte{0xb4},
    byte{0x3c}, byte{0x6e}, byte{0x0a}, byte{0xb9},
    byte{0x2e}, byte{0x87}, byte{0x0e}, byte{0xdb},
    byte{0x2d}, byte{0xe0}, byte{0xa8}, byte{0x8c},
    byte{0xae}, byte{0x12}, byte{0xdb}, byte{0xd8},
    byte{0x59}, byte{0x15}, byte{0x07}, byte{0xf5},
    byte{0x84}, byte{0xfe}, byte{0x49}, byte{0x12},
    byte{0xba}, byte{0xbf}, byte{0xf4}, byte{0x97},
    byte{0xf1}, byte{0xb8}, byte{0xed}, byte{0xf9},
    byte{0x56}, byte{0x7d}, byte{0x24}, byte{0x83},
    byte{0xd5}, byte{0x4d}, byte{0xdc}, byte{0x64},
    byte{0x59}, byte{0xbe}, byte{0xa7}, byte{0x85},
    byte{0x52}, byte{0x81}, byte{0xb7}, byte{0xa2},
    byte{0x46}, byte{0xa6}, byte{0x09}, byte{0xe3},
    byte{0x00}, byte{0x1a}, byte{0x4e}, byte{0x08},
  };
  EXPECT_FALSE(key->verify(message, signature));
}

TEST(KeyPairEdTests, TestFailsToVerifyIncorrectSignature)
{
  const auto key_data = vector<byte>{
    byte{0xb7}, byte{0xa3}, byte{0xc1}, byte{0x2d},
    byte{0xc0}, byte{0xc8}, byte{0xc7}, byte{0x48},
    byte{0xab}, byte{0x07}, byte{0x52}, byte{0x5b},
    byte{0x70}, byte{0x11}, byte{0x22}, byte{0xb8},
    byte{0x8b}, byte{0xd7}, byte{0x8f}, byte{0x60},
    byte{0x0c}, byte{0x76}, byte{0x34}, byte{0x2d},
    byte{0x27}, byte{0xf2}, byte{0x5e}, byte{0x5f},
    byte{0x92}, byte{0x44}, byte{0x4c}, byte{0xde},
  };
  const auto key = KeyPair::create_ed25519_pub(key_data);
  const auto message = vector<byte>{
    byte{'M'}, byte{'e'}, byte{'s'}, byte{'s'},
    byte{'a'}, byte{'g'}, byte{'e'}, byte{' '},
    byte{'f'}, byte{'o'}, byte{'r'}, byte{' '},
    byte{'E'}, byte{'d'}, byte{'2'}, byte{'5'},
    byte{'5'}, byte{'1'}, byte{'9'}, byte{' '},
    byte{'s'}, byte{'i'}, byte{'g'}, byte{'n'},
    byte{'i'}, byte{'n'}, byte{'g'},
  };
  const auto signature = vector<byte>{
    byte{0xde}, byte{0xad}, byte{0xbe}, byte{0xef},
    byte{0x7f}, byte{0xae}, byte{0x4e}, byte{0xb4},
    byte{0x3c}, byte{0x6e}, byte{0x0a}, byte{0xb9},
    byte{0x2e}, byte{0x87}, byte{0x0e}, byte{0xdb},
    byte{0x2d}, byte{0xe0}, byte{0xa8}, byte{0x8c},
    byte{0xae}, byte{0x12}, byte{0xdb}, byte{0xd8},
    byte{0x59}, byte{0x15}, byte{0x07}, byte{0xf5},
    byte{0x84}, byte{0xfe}, byte{0x49}, byte{0x12},
    byte{0xba}, byte{0xbf}, byte{0xf4}, byte{0x97},
    byte{0xf1}, byte{0xb8}, byte{0xed}, byte{0xf9},
    byte{0x56}, byte{0x7d}, byte{0x24}, byte{0x83},
    byte{0xd5}, byte{0x4d}, byte{0xdc}, byte{0x64},
    byte{0x59}, byte{0xbe}, byte{0xa7}, byte{0x85},
    byte{0x52}, byte{0x81}, byte{0xb7}, byte{0xa2},
    byte{0x46}, byte{0xa6}, byte{0x09}, byte{0xe3},
    byte{0x00}, byte{0x1a}, byte{0x4e}, byte{0x08},
  };
  EXPECT_FALSE(key->verify(message, signature));
}

TEST(KeyPairEdTests, TestVerfiesCorrectSignature)
{
  const auto key_data = vector<byte>{
    byte{0xb7}, byte{0xa3}, byte{0xc1}, byte{0x2d},
    byte{0xc0}, byte{0xc8}, byte{0xc7}, byte{0x48},
    byte{0xab}, byte{0x07}, byte{0x52}, byte{0x5b},
    byte{0x70}, byte{0x11}, byte{0x22}, byte{0xb8},
    byte{0x8b}, byte{0xd7}, byte{0x8f}, byte{0x60},
    byte{0x0c}, byte{0x76}, byte{0x34}, byte{0x2d},
    byte{0x27}, byte{0xf2}, byte{0x5e}, byte{0x5f},
    byte{0x92}, byte{0x44}, byte{0x4c}, byte{0xde},
  };
  const auto key = KeyPair::create_ed25519_pub(key_data);
  const auto message = vector<byte>{
    byte{'M'}, byte{'e'}, byte{'s'}, byte{'s'},
    byte{'a'}, byte{'g'}, byte{'e'}, byte{' '},
    byte{'f'}, byte{'o'}, byte{'r'}, byte{' '},
    byte{'E'}, byte{'d'}, byte{'2'}, byte{'5'},
    byte{'5'}, byte{'1'}, byte{'9'}, byte{' '},
    byte{'s'}, byte{'i'}, byte{'g'}, byte{'n'},
    byte{'i'}, byte{'n'}, byte{'g'},
  };
  const auto signature = vector<byte>{
    byte{0x6d}, byte{0xd3}, byte{0x55}, byte{0x66},
    byte{0x7f}, byte{0xae}, byte{0x4e}, byte{0xb4},
    byte{0x3c}, byte{0x6e}, byte{0x0a}, byte{0xb9},
    byte{0x2e}, byte{0x87}, byte{0x0e}, byte{0xdb},
    byte{0x2d}, byte{0xe0}, byte{0xa8}, byte{0x8c},
    byte{0xae}, byte{0x12}, byte{0xdb}, byte{0xd8},
    byte{0x59}, byte{0x15}, byte{0x07}, byte{0xf5},
    byte{0x84}, byte{0xfe}, byte{0x49}, byte{0x12},
    byte{0xba}, byte{0xbf}, byte{0xf4}, byte{0x97},
    byte{0xf1}, byte{0xb8}, byte{0xed}, byte{0xf9},
    byte{0x56}, byte{0x7d}, byte{0x24}, byte{0x83},
    byte{0xd5}, byte{0x4d}, byte{0xdc}, byte{0x64},
    byte{0x59}, byte{0xbe}, byte{0xa7}, byte{0x85},
    byte{0x52}, byte{0x81}, byte{0xb7}, byte{0xa2},
    byte{0x46}, byte{0xa6}, byte{0x09}, byte{0xe3},
    byte{0x00}, byte{0x1a}, byte{0x4e}, byte{0x08},
  };
  EXPECT_TRUE(key->verify(message, signature));
}

TEST(KeyPairEdTests, TestSignsMessageCorrectly)
{
  const auto key_data = vector<byte>{
    byte{0x14}, byte{0x98}, byte{0xb5}, byte{0x46}, byte{0x7a}, byte{0x63},
    byte{0xdf}, byte{0xfa}, byte{0x2d}, byte{0xc9}, byte{0xd9}, byte{0xe0},
    byte{0x69}, byte{0xca}, byte{0xf0}, byte{0x75}, byte{0xd1}, byte{0x6f},
    byte{0xc3}, byte{0x3f}, byte{0xdd}, byte{0x4c}, byte{0x3b}, byte{0x01},
    byte{0xbf}, byte{0xad}, byte{0xae}, byte{0x64}, byte{0x33}, byte{0x76},
    byte{0x7d}, byte{0x93},
  };
  const auto key = KeyPair::create_ed25519(key_data);
  const auto message = vector<byte>{
    byte{'M'}, byte{'e'}, byte{'s'}, byte{'s'},
    byte{'a'}, byte{'g'}, byte{'e'}, byte{' '},
    byte{'f'}, byte{'o'}, byte{'r'}, byte{' '},
    byte{'E'}, byte{'d'}, byte{'2'}, byte{'5'},
    byte{'5'}, byte{'1'}, byte{'9'}, byte{' '},
    byte{'s'}, byte{'i'}, byte{'g'}, byte{'n'},
    byte{'i'}, byte{'n'}, byte{'g'},
  };
  const auto signature = key->sign(message);
  const auto expected = vector<byte>{
    byte{0x6d}, byte{0xd3}, byte{0x55}, byte{0x66},
    byte{0x7f}, byte{0xae}, byte{0x4e}, byte{0xb4},
    byte{0x3c}, byte{0x6e}, byte{0x0a}, byte{0xb9},
    byte{0x2e}, byte{0x87}, byte{0x0e}, byte{0xdb},
    byte{0x2d}, byte{0xe0}, byte{0xa8}, byte{0x8c},
    byte{0xae}, byte{0x12}, byte{0xdb}, byte{0xd8},
    byte{0x59}, byte{0x15}, byte{0x07}, byte{0xf5},
    byte{0x84}, byte{0xfe}, byte{0x49}, byte{0x12},
    byte{0xba}, byte{0xbf}, byte{0xf4}, byte{0x97},
    byte{0xf1}, byte{0xb8}, byte{0xed}, byte{0xf9},
    byte{0x56}, byte{0x7d}, byte{0x24}, byte{0x83},
    byte{0xd5}, byte{0x4d}, byte{0xdc}, byte{0x64},
    byte{0x59}, byte{0xbe}, byte{0xa7}, byte{0x85},
    byte{0x52}, byte{0x81}, byte{0xb7}, byte{0xa2},
    byte{0x46}, byte{0xa6}, byte{0x09}, byte{0xe3},
    byte{0x00}, byte{0x1a}, byte{0x4e}, byte{0x08},
  };
  EXPECT_EQ(expected, signature);
}
#endif

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
