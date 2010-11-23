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

namespace ObTools { namespace Crypto {

//------------------------------------------------------------------------
// Encrypt/decrypt a block in place
// If block is not padded to 8 bytes, the remainder (up to 7) bytes 
// WILL NOT BE ENCRYPTED
// Encrypts if 'encryption' is set (default), otherwise decrypts
// IV is modified if set
// Returns whether successful (keys set up correctly)
bool AES::encrypt(unsigned char *data, int length, bool encryption)
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
    length = AES_BLOCK_SIZE * (length / AES_BLOCK_SIZE);

    // CBC
    AES_cbc_encrypt(data, data, length, &aes_key, &iv.key[0], enc);
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

}} // namespaces




