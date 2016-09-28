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

TEST(AESStringTests, TestEncryptDecryptKey)
{
  // Create random 'session' key and iv
  Crypto::AESKey session_key(Crypto::AESKey::BITS_128);
  session_key.create();

  Crypto::AESKey session_iv(Crypto::AESKey::BITS_128, 0);
  session_iv.create();

  // Create 'content' key to be encrypted (must be 128)
  Crypto::AESKey content_key(Crypto::AESKey::BITS_128);
  content_key.create();
  Crypto::AESKey content_key_original = content_key;

  // Create encryptor, CBC
  Crypto::AES aes;
  aes.set_key(session_key);
  aes.set_iv(session_iv);

  ASSERT_TRUE(aes.encrypt(content_key));
  ASSERT_EQ(content_key.size, Crypto::AESKey::BITS_256);

  aes.set_iv(session_iv);  // reset IV
  ASSERT_TRUE(aes.decrypt(content_key));
  ASSERT_EQ(content_key.size, Crypto::AESKey::BITS_128);

  ASSERT_EQ(content_key_original.str(), content_key.str());
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
