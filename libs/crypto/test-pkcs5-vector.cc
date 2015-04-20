//==========================================================================
// ObTools::Crypto: test-pkcs5-vector.cc
//
// Test harness for Crypto library PKCS5 padding functions, vector versions
//
// Copyright (c) 2015 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-crypto.h"

using namespace std;
using namespace ObTools;

TEST(AESKeyTests, TestPadShortString)
{
  for(int i=0; i<8; i++)
  {
    vector<unsigned char> data;
    if (i) data.insert(data.end(), i, 0x42U);
    Crypto::PKCS5::pad(data, 8);
    ASSERT_EQ(8, data.size()) << "padded length bad at size " << i;
    for(int j=i; j<8; j++)
      EXPECT_EQ(8-i, data[j]) << "padding char bad at size " << i;
  }
}

TEST(AESKeyTests, TestUnpadShortString)
{
  for(int i=0; i<8; i++)
  {
    vector<unsigned char> expected;
    for(int j=0; j<i; j++) expected.insert(expected.end(), 42+j);

    vector<unsigned char> data = expected;
    data.insert(data.end(), 8-i, 8-i);

    Crypto::PKCS5::unpad(data);
    EXPECT_EQ(expected, data) << "Unpad mismatch at size " << i;
  }
}

TEST(AESKeyTests, TestPadExactLengthString)
{
  vector<unsigned char> data;
  data.insert(data.end(), 8, 0x42U);
  Crypto::PKCS5::pad(data, 8);
  ASSERT_EQ(16, data.size());
  for(int i=8; i<16; i++)
    EXPECT_EQ(8, data[i]);
}

TEST(AESKeyTests, TestUnpadExactLengthString)
{
  vector<unsigned char> expected;
  for(int i=0; i<8; i++) expected.insert(expected.end(), 42+i);

  vector<unsigned char> data = expected;
  data.insert(data.end(), 8, 8);
  Crypto::PKCS5::unpad(data);
  EXPECT_EQ(expected, data);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
