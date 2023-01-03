//==========================================================================
// ObTools::Crypto: sha256.cc
//
// SHA256 digest/hash
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <stdlib.h>
#include "ot-crypto.h"
#include "ot-text.h"
#include <sstream>
#include <iomanip>

// Temporary bodge to ignore deprecations in OpenSSL 3.0.
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

namespace ObTools { namespace Crypto {

//--------------------------------------------------------------------------
// Constructor
SHA256::SHA256(): finished(false)
{
  SHA256_Init(&sha_ctx);
}

//--------------------------------------------------------------------------
// Update digest with a block of data
void SHA256::update(const void *data, size_t length)
{
  if (!finished) SHA256_Update(&sha_ctx, data,
                             static_cast<unsigned long>(length));
}

//--------------------------------------------------------------------------
// Get result - writes DIGEST_LENGTH bytes to result
void SHA256::get_result(unsigned char *result)
{
  if (!finished) SHA256_Final(result, &sha_ctx);
  finished = true;
}

//--------------------------------------------------------------------------
// Get result as a hex string
string SHA256::get_result()
{
  if (!finished)
  {
    unsigned char buf[DIGEST_LENGTH];
    SHA256_Final(buf, &sha_ctx);
    finished = true;
    return Text::btox(buf, DIGEST_LENGTH);
  }
  else throw logic_error("Reused SHA256 after get_result()");
}

//--------------------------------------------------------------------------
// Destructor
SHA256::~SHA256()
{
  if (!finished)
  {
    // Dump into a fake buffer
    unsigned char bucket[DIGEST_LENGTH];
    SHA256_Final(bucket, &sha_ctx);
  }
}

//--------------------------------------------------------------------------
// Static: Get hash of block of data.  Writes DIGEST_LENGTH bytes to result
void SHA256::digest(const void *data, size_t length, unsigned char *result)
{
  ::SHA256(static_cast<const unsigned char *>(data),
         static_cast<unsigned long>(length), result);
}

//--------------------------------------------------------------------------
// Digest returning binary string
string SHA256::digest(const void *data, size_t length)
{
  unsigned char buf[DIGEST_LENGTH];
  digest(data, length, buf);
  return string(reinterpret_cast<char *>(buf), DIGEST_LENGTH);
}

//--------------------------------------------------------------------------
// Digest returning hex string
string SHA256::digest_hex(const void *data, size_t length)
{
  unsigned char buf[DIGEST_LENGTH];
  digest(data, length, buf);
  return Text::btox(buf, DIGEST_LENGTH);
}

}} // namespaces
