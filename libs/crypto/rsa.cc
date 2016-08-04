//==========================================================================
// ObTools::Crypto: rsa.cc
//
// RSA encryption/decryption
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <stdlib.h>
#include "ot-crypto.h"

namespace ObTools { namespace Crypto {

//------------------------------------------------------------------------
// Get ciphertext size
int RSA::cipher_size()
{
  return key.valid?RSA_size(key.rsa):0;
}

//------------------------------------------------------------------------
// Get maximum plaintext size
int RSA::max_plaintext()
{
  // Allow for PKCS1 padding
  return key.valid?RSA_size(key.rsa)-11:0;
}

//------------------------------------------------------------------------
// Encrypt a block
// Length may be up to max_plaintext() bytes
// Returns whether successful (key set up correctly)
// 'to' must be writable to cipher_size() bytes
bool RSA::encrypt(const unsigned char *from, int length,
		  unsigned char *to)
{
  if (!key.valid) return false;

  int res;
  if (key.is_private)
    res = RSA_private_encrypt(length, from, to, key.rsa, RSA_PKCS1_PADDING);
  else
    res = RSA_public_encrypt(length, from, to, key.rsa, RSA_PKCS1_PADDING);

  return res>=0;
}

//------------------------------------------------------------------------
// Decrypt a block
// Returns decrypted length of block
// Assumes 'from' data is always cipher_size() bytes
// 'to' must be writable to cipher_size() bytes
// (unless you _really_ understand padding!)
int RSA::decrypt(const unsigned char *from, unsigned char *to)
{
  if (!key.valid) return false;

  int res;
  int length = cipher_size();
  if (key.is_private)
    res = RSA_private_decrypt(length, from, to, key.rsa, RSA_PKCS1_PADDING);
  else
    res = RSA_public_decrypt(length, from, to, key.rsa, RSA_PKCS1_PADDING);

  return (res<0)?0:res;
}

}} // namespaces

