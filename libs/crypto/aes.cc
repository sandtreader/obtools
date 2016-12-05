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

//--------------------------------------------------------------------------
// Encrypt/decrypt a block in place
bool AES::encrypt(unsigned char *data, int length, bool encryption, bool rtb)
{
  AES_KEY aes_key;
  int enc;
  // Save the initial IV for residual termination block
  AESKey initial_iv(iv);

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

    unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>
      ctx{EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free};
    EVP_EncryptInit_ex(ctx.get(), EVP_aes_128_ctr(), nullptr, key.key, iv.key);
    auto enc_len = int{};
    EVP_EncryptUpdate(ctx.get(), data, &enc_len, data, length);
    EVP_EncryptFinal_ex(ctx.get(), data + enc_len, &enc_len);
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
        AES_cbc_encrypt(p, last, sizeof(last), &aes_key,
                        &initial_iv.key[0], AES_ENCRYPT);
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

//--------------------------------------------------------------------------
// Sugared version of encrypt with binary strings and PKCS7 padding
bool AES::encrypt(const string& plaintext, string& ciphertext_p)
{
  int length = plaintext.size();

  // Copy to safe buffer and pad
  vector<unsigned char> pt(length);
  memcpy(&pt[0], plaintext.data(), length);

  PKCS7::pad(pt, AES_BLOCK_SIZE);
  length = pt.size();

  // Encrypt
  if (!encrypt(&pt[0], length)) return false;

  // Convert back to string
  ciphertext_p = string(reinterpret_cast<const char *>(&pt[0]), length);

  return true;
}

//--------------------------------------------------------------------------
// Sugared version of encrypt, encrypting and PKCS7 padding in place
bool AES::encrypt(vector<unsigned char>& data)
{
  PKCS7::pad(data, AES_BLOCK_SIZE);
  return encrypt(&data[0], data.size());
}

//--------------------------------------------------------------------------
// Sugared version of decrypt with binary  strings and PKCS7 unpadding
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
  length = PKCS7::original_length(data, length);

  // Convert back to string
  plaintext_p = string(reinterpret_cast<const char *>(data), length);

  return true;
}

//--------------------------------------------------------------------------
// Sugared version of decrypt, PKCS7 unpadding and decrypting in place
bool AES::decrypt(vector<unsigned char>& data)
{
  if (!decrypt(&data[0], data.size())) return false;
  PKCS7::unpad(data);
  return true;
}

//--------------------------------------------------------------------------
// Encrypt another key in place, with padding
// Only works for 128-bit keys, turns into a 256 bit 'key' for output
bool AES::encrypt(AESKey& key)
{
  if (key.size != AESKey::BITS_128) return false;
  PKCS7::pad_in_place(key.key, 16, 16); // To 32 bytes
  key.size = AESKey::BITS_256;
  if (!encrypt(key.key, 32)) return false;
  return true;
}

//--------------------------------------------------------------------------
// Decrypt a key in place, unpadding
// Only works for 128-bit keys encrypted to 256 bits
bool AES::decrypt(AESKey& key)
{
  if (key.size != AESKey::BITS_256) return false;
  if (!encrypt(key.key, 32, false)) return false;
  key.size = AESKey::BITS_128;  // Simply truncate
  return true;
}

}} // namespaces




