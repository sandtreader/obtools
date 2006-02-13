//==========================================================================
// ObTools::Crypto: rsa.cc
//
// RSA encryption/decryption
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include <stdlib.h>
#include "ot-crypto.h"

namespace ObTools { namespace Crypto {

//------------------------------------------------------------------------
// Get cyphertext size
int RSA::cypher_size()
{
  return key.valid?OpenSSL::RSA_size(key.rsa):0;
}

//------------------------------------------------------------------------
// Get maximum plaintext size
int RSA::max_plaintext()
{
  // Allow for PKCS1 padding
  return key.valid?OpenSSL::RSA_size(key.rsa)-11:0;
}

//------------------------------------------------------------------------
// Encrypt a block 
// Length may be up to max_plaintext() bytes
// Returns whether successful (key set up correctly)
// 'to' must be writable to cypher_size() bytes
bool RSA::encrypt(const unsigned char *from, int length, 
		  unsigned char *to)
{
  if (!key.valid) return false;

  int res;
  if (key.is_private)
    res = OpenSSL::RSA_private_encrypt(length, from, to, key.rsa, 
				       RSA_PKCS1_PADDING);
  else
    res = OpenSSL::RSA_public_encrypt(length, from, to, key.rsa, 
				      RSA_PKCS1_PADDING);

  return res>=0;
}

//------------------------------------------------------------------------
// Decrypt a block 
// Returns decrypted length of block
// Assumes 'from' data is always cypher_size() bytes
// 'to' must be writable to cypher_size() bytes 
// (unless you _really_ understand padding!)
int RSA::decrypt(const unsigned char *from, unsigned char *to)
{
  if (!key.valid) return false;

  int res;
  int length = cypher_size();
  if (key.is_private)
    res = OpenSSL::RSA_private_decrypt(length, from, to, key.rsa, 
				       RSA_PKCS1_PADDING);
  else
    res = OpenSSL::RSA_public_decrypt(length, from, to, key.rsa, 
				      RSA_PKCS1_PADDING);

  return (res<0)?0:res;
}

}} // namespaces
