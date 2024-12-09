//==========================================================================
// ObTools::Crypto: test-hash.cc
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

TEST(HashTests, TestProducesCorrectRIPEMD160Hash)
{
  const auto data = vector<byte>{
    byte{'H'}, byte{'e'}, byte{'l'}, byte{'l'}, byte{'o'}, byte{','},
    byte{' '}, byte{'w'}, byte{'o'}, byte{'r'}, byte{'l'}, byte{'d'},
    byte{'!'},
  };
  const auto expected = vector<byte>{
    byte{0x58}, byte{0x26}, byte{0x2d}, byte{0x1f}, byte{0xbd}, byte{0xbe},
    byte{0x45}, byte{0x30}, byte{0xd8}, byte{0x86}, byte{0x5d}, byte{0x35},
    byte{0x18}, byte{0xc6}, byte{0xd6}, byte{0xe4}, byte{0x10}, byte{0x02},
    byte{0x61}, byte{0x0f},
  };
  EXPECT_EQ(expected, Hash::ripemd160(data));
}

TEST(HashTests, TestProducesCorrectSHA3_256Hash)
{
  const auto data = vector<byte>{
    byte{'H'}, byte{'e'}, byte{'l'}, byte{'l'}, byte{'o'}, byte{','},
    byte{' '}, byte{'w'}, byte{'o'}, byte{'r'}, byte{'l'}, byte{'d'},
    byte{'!'},
  };
  const auto expected = vector<byte>{
    byte{0xf3}, byte{0x45}, byte{0xa2}, byte{0x19}, byte{0xda}, byte{0x00},
    byte{0x5e}, byte{0xbe}, byte{0x9c}, byte{0x1a}, byte{0x1e}, byte{0xaa},
    byte{0xd9}, byte{0x7b}, byte{0xbf}, byte{0x38}, byte{0xa1}, byte{0x0c},
    byte{0x84}, byte{0x73}, byte{0xe4}, byte{0x1d}, byte{0x0a}, byte{0xf7},
    byte{0xfb}, byte{0x61}, byte{0x7c}, byte{0xaa}, byte{0x0c}, byte{0x6a},
    byte{0xa7}, byte{0x22},
  };
  EXPECT_EQ(expected, Hash::sha3_256(data));
}

#if OPENSSL_VERSION_MAJOR > 3 || \
    (OPENSSL_VERSION_MAJOR == 3 && OPENSSL_VERSION_MINOR >= 2)
TEST(HashTests, TestProducesCorrectKECCAK256Hash)
{
  const auto data = vector<byte>{
    byte{'H'}, byte{'e'}, byte{'l'}, byte{'l'}, byte{'o'}, byte{','},
    byte{' '}, byte{'w'}, byte{'o'}, byte{'r'}, byte{'l'}, byte{'d'},
    byte{'!'},
  };
  const auto expected = vector<byte>{
    byte{0xb6}, byte{0xe1}, byte{0x6d}, byte{0x27}, byte{0xac}, byte{0x5a},
    byte{0xb4}, byte{0x27}, byte{0xa7}, byte{0xf6}, byte{0x89}, byte{0x00},
    byte{0xac}, byte{0x55}, byte{0x59}, byte{0xce}, byte{0x27}, byte{0x2d},
    byte{0xc6}, byte{0xc3}, byte{0x7c}, byte{0x82}, byte{0xb3}, byte{0xe0},
    byte{0x52}, byte{0x24}, byte{0x6c}, byte{0x82}, byte{0x24}, byte{0x4c},
    byte{0x50}, byte{0xe4},
  };
  EXPECT_EQ(expected, Hash::keccak256(data));
}
#endif

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
