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
#include "ot-text.h"

// Temporary bodge to ignore deprecation of HMAC_xx in OpenSSL 3.0.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

namespace ObTools { namespace Crypto {

//--------------------------------------------------------------------------
// Constructor
HMAC::HMAC(const void *key, int key_len, const EVP_MD *md,
           unsigned int _digest_length):
  finished(false), digest_length(_digest_length)
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  HMAC_CTX_init(hmac_ctx.get());
#endif
  HMAC_Init_ex(hmac_ctx.get(), key, key_len, md, 0);
}

//--------------------------------------------------------------------------
// Update digest with a block of data
void HMAC::update(const unsigned char *data, size_t length)
{
  if (!finished)
    HMAC_Update(hmac_ctx.get(), data, length);
}

//--------------------------------------------------------------------------
// Get result - writes digest_length() bytes to result
void HMAC::get_result(unsigned char *result)
{
  if (!finished)
  {
    unsigned int len;
    HMAC_Final(hmac_ctx.get(), result, &len);
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
// Digest returning binary string
string HMAC::digest(const unsigned char *data, size_t length)
{
  vector<unsigned char> buf(digest_length);
  digest(data, length, buf.data());
  return string(reinterpret_cast<char *>(buf.data()), digest_length);
}

//--------------------------------------------------------------------------
// Digest returning hex string
string HMAC::digest_hex(const unsigned char *data, size_t length)
{
  vector<unsigned char> buf(digest_length);
  digest(data, length, buf.data());
  return Text::btox(buf.data(), digest_length);
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
    HMAC_Final(hmac_ctx.get(), &bucket[0], &len);
  }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
  HMAC_CTX_cleanup(hmac_ctx.get());
#endif
}

}} // namespaces
