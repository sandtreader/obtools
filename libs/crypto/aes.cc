//==========================================================================
// ObTools::Crypto: aes.cc
//
// AES encryption/decryption
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <stdlib.h>
#include "ot-crypto.h"
#include <memory>

namespace ObTools { namespace Crypto {

//------------------------------------------------------------------------
// Encrypt/decrypt a block in place
bool AES::encrypt(unsigned char *data, int length, bool encryption, bool rtb)
{
  AES_KEY aes_key;
  int enc;

  // Set up AES key
  if (ctr || encryption) {
    AES_set_encrypt_key(&key.key[0], key.size, &aes_key);
    enc = AES_ENCRYPT;
  }
  else
  {
    AES_set_decrypt_key(&key.key[0], key.size, &aes_key);
    enc = AES_DECRYPT;
  }

  if (ctr)
  {
    // CTR
    if (key.size != AESKey::BITS_128 || !iv.valid)
      return false;

    unsigned char counter[AES_BLOCK_SIZE];
    memset(counter, 0, sizeof(counter));
    unsigned int num = 0;
    AES_ctr128_encrypt(data, data, length, &aes_key, iv.key, counter, &num);
  }
  else if (iv.valid) // Check for CBC - IV is valid
  {
    // Round length down to block size multiple
    int enc_length = AES_BLOCK_SIZE * (length / AES_BLOCK_SIZE);
    unsigned char dec_last_full[AES_BLOCK_SIZE];

    // Residual termination block if requested
    if (!encryption && rtb && length > enc_length)
    {
      memcpy(dec_last_full, data + enc_length - AES_BLOCK_SIZE, AES_BLOCK_SIZE);
    }

    // CBC
    AES_cbc_encrypt(data, data, enc_length, &aes_key, &iv.key[0], enc);

    // Residual termination block if requested (encryption)
    if (rtb && length > enc_length)
    {
      unsigned char last[AES_BLOCK_SIZE];
      unsigned char *p = encryption
                         ? (data + enc_length - AES_BLOCK_SIZE)
                         : dec_last_full;
      AES_set_encrypt_key(&key.key[0], key.size, &aes_key);
      if (enc_length)
      {
        AESKey rtb_iv(iv);
        AES_cbc_encrypt(p, last, sizeof(last), &aes_key,
                        &rtb_iv.key[0], AES_ENCRYPT);
      }
      else
      {
        AES_ecb_encrypt(&short_rand.key[0], last, &aes_key, AES_ENCRYPT);
      }
      unsigned char *final = data + enc_length;
      unsigned char *end = data + length;
      unsigned char *last_p = last;
      while (final != end)
        *final++ ^= *last_p++;
    }
  }
  else
  {
    // Round length down to block size multiple
    length = AES_BLOCK_SIZE * (length / AES_BLOCK_SIZE);

    // ECB
    for (int i = 0; i < length; i += AES_BLOCK_SIZE, data += AES_BLOCK_SIZE)
    {
      AES_ecb_encrypt(data, data, &aes_key, enc);
    }
  }

  return true;
}

//------------------------------------------------------------------------
// Sugared version of encrypt with binary strings and PKCS5 padding
bool AES::encrypt(const string& plaintext, string& ciphertext_p)
{
  // Pad token to AES_BLOCK_SIZE
  int length = plaintext.size();
  auto_ptr<unsigned char> padded(PKCS5::pad(
	     reinterpret_cast<const unsigned char *>(plaintext.data()), 
	     length, AES_BLOCK_SIZE));

  // Encrypt
  if (!encrypt(padded.get(), length)) return false;

  // Convert back to string
  ciphertext_p = string(reinterpret_cast<const char *>(padded.get()), length);

  return true;
}

//------------------------------------------------------------------------
// Sugared version of decrypt with binary  strings and PKCS5 unpadding
bool AES::decrypt(const string& ciphertext, string& plaintext_p)
{
  int length = ciphertext.size();

  // Copy to safe buffer
  vector<unsigned char> ct(length);
  unsigned char *data = &ct[0];
  memcpy(data, ciphertext.data(), length);

  // Decrypt
  if (!decrypt(data, length)) return false;

  // Unpad
  length = PKCS5::original_length(data, length);

  // Convert back to string
  plaintext_p = string(reinterpret_cast<const char *>(data), length);

  return true;
}

}} // namespaces




