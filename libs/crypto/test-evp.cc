//==========================================================================
// ObTools::Crypto: test-evp.cc
//
// Test harness for Crypto library EVP crypto functions
//
// Copyright (c) 2024 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-crypto.h"

using namespace std;
using namespace ObTools;
using namespace ObTools::Crypto;

TEST(EVPTests, TestFailsToInitialiseInvalidKey)
{
  const auto key_data = vector<byte>{
    byte{0x00}
  };
  const auto key = EVPKey(EVPKey::Type::Ed25519, key_data);
  EXPECT_FALSE(key.is_valid());
}

TEST(EVPTests, TestInitialisesValidKey)
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
  const auto key = EVPKey(EVPKey::Type::Ed25519, key_data);
  EXPECT_TRUE(key.is_valid());
}

TEST(EVPTests, TestFailsForIncorrectSignature)
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
  const auto key = EVPKey(EVPKey::Type::Ed25519, key_data);
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
  EXPECT_FALSE(EVP::verify(key, message, signature));
}

TEST(EVPTests, TestFailsToVerifyIncorrectSignature)
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
  const auto key = EVPKey(EVPKey::Type::Ed25519, key_data);
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
  EXPECT_FALSE(EVP::verify(key, message, signature));
}

TEST(EVPTests, TestVerfiesCorrectSignature)
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
  const auto key = EVPKey(EVPKey::Type::Ed25519, key_data);
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
  EXPECT_TRUE(EVP::verify(key, message, signature));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
