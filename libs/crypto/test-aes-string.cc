//==========================================================================
// ObTools::Crypto: test-aes-string.cc
//
// Test harness for Crypto library AES encryption/decryption functions
// string versions
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-crypto.h"
#include "ot-misc.h"
#include <iostream>

#define PASSPHRASE "Mary had a little lamb"

using namespace std;
using namespace ObTools;

TEST(AESKeyTests, TestSetAESKeyFromInteger)
{
  Crypto::AESKey key(Crypto::AESKey::BITS_128);
  key.set_from_int(42);
  ASSERT_TRUE(key.valid);
  ASSERT_EQ(key.str(), "0000000000000000000000000000002a");
}

TEST(AESKeyTests, TestBase64ReadWrite)
{
  Crypto::AESKey key(Crypto::AESKey::BITS_128);
  key.create();
  string b64 = key.str_base64();

  Crypto::AESKey key2(Crypto::AESKey::BITS_128);
  ASSERT_TRUE(key2.set_from_base64(b64));
  ASSERT_TRUE(key.valid);
  ASSERT_EQ(key.str(), key2.str());
}

TEST(AESStringTests, TestEncryptDecryptCycleMatches)
{
  // Create a key from a known passphrase
  Crypto::AESKey key(Crypto::AESKey::BITS_256);
  key.set_from_passphrase(PASSPHRASE);
  ASSERT_TRUE(key.valid);

  // Create encryptor, ECB
  Crypto::AES aes;
  aes.set_key(key);
  ASSERT_FALSE(aes.iv.valid);

  string plain = "Hello, world!";  // Note, 13 bytes
  string cipher;

  ASSERT_TRUE(aes.encrypt(plain, cipher));
  ASSERT_EQ(cipher.size(), 16);  // Must be padded

  string decrypted;
  ASSERT_TRUE(aes.decrypt(cipher, decrypted));
  ASSERT_EQ(plain, decrypted);
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
