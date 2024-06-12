//==========================================================================
// ObTools::Text: test-bech32.cc
//
// GTest harness for text library Bech32 functions
//
// Copyright (c) 2024 Paul Clark.
//==========================================================================

#include "ot-text.h"
#include <gtest/gtest.h>
#include <algorithm>

namespace {

using namespace std;
using namespace ObTools;

TEST(Bech32Test, TestEncode)
{
  const auto data = vector<byte>{
    byte{0x99}, byte{0x91}, byte{0x95}, byte{0x7d}, byte{0x7a}, byte{0xc1},
    byte{0xe8}, byte{0xf7}, byte{0x11}, byte{0x75}, byte{0xf4}, byte{0xe0},
    byte{0xad}, byte{0xe8}, byte{0x77}, byte{0xfe}, byte{0x87}, byte{0xb0},
    byte{0x21}, byte{0x86}, byte{0x4c}, byte{0x30}, byte{0x1d}, byte{0x16},
    byte{0xc7}, byte{0x9a}, byte{0x49}, byte{0xaf}, byte{0x06}, byte{0x25},
    byte{0x09}, byte{0x3f},
  };
  const auto expected = string{
    "nxge2lt6c850wyt47ns2m6rhl6rmqgvxfscp69k8nfy67p39pyls"
  };
  EXPECT_EQ(expected, Text::Bech32::encode(data));
}

TEST(Bech32Test, TestDecode)
{
  const auto expected = vector<byte>{
    byte{0x99}, byte{0x91}, byte{0x95}, byte{0x7d}, byte{0x7a}, byte{0xc1},
    byte{0xe8}, byte{0xf7}, byte{0x11}, byte{0x75}, byte{0xf4}, byte{0xe0},
    byte{0xad}, byte{0xe8}, byte{0x77}, byte{0xfe}, byte{0x87}, byte{0xb0},
    byte{0x21}, byte{0x86}, byte{0x4c}, byte{0x30}, byte{0x1d}, byte{0x16},
    byte{0xc7}, byte{0x9a}, byte{0x49}, byte{0xaf}, byte{0x06}, byte{0x25},
    byte{0x09}, byte{0x3f},
  };
  const auto data = string{
    "nxge2lt6c850wyt47ns2m6rhl6rmqgvxfscp69k8nfy67p39pyls"
  };
  auto actual = vector<byte>{};
  ASSERT_TRUE(Text::Bech32::decode(data, actual));
  EXPECT_EQ(expected, actual);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
