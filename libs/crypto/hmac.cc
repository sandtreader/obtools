//==========================================================================
// ObTools::Crypto: hmac.cc
//
// HMAC implementation
//
// Copyright (c) 2012 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <stdlib.h>
#include "ot-crypto.h"

namespace ObTools { namespace Crypto {

//--------------------------------------------------------------------------
// Constructor
HMAC::HMAC(const void *key, int key_len, const EVP_MD *md,
           unsigned int _digest_length):
  finished(false), digest_length(_digest_length)
{
  HMAC_CTX_init(&hmac_ctx);
  HMAC_Init_ex(&hmac_ctx, key, key_len, md, 0);
}

//--------------------------------------------------------------------------
// Update digest with a block of data
void HMAC::update(const unsigned char *data, size_t length)
{
  if (!finished)
    HMAC_Update(&hmac_ctx, data, length);
}

//--------------------------------------------------------------------------
// Get result - writes digest_length() bytes to result
void HMAC::get_result(unsigned char *result)
{
  if (!finished)
  {
    unsigned int len;
    HMAC_Final(&hmac_ctx, result, &len);
  }
  finished = true;
}

//--------------------------------------------------------------------------
// Get hash of block of data.  Writes get_digest_length() bytes to result
void HMAC::digest(const unsigned char *data, size_t length,
                  unsigned char *result)
{
  update(data, length);
  get_result(result);
}

//--------------------------------------------------------------------------
// Virtual Destructor
HMAC::~HMAC()
{
  if (!finished)
  {
    // Dump into a fake buffer
    auto bucket = vector<unsigned char>(digest_length);
    unsigned int len;
    HMAC_Final(&hmac_ctx, &bucket[0], &len);
  }
}

}} // namespaces
