//==========================================================================
// ObTools::Crypto: sha1.cc
//
// SHA1 digest/hash
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <stdlib.h>
#include "ot-crypto.h"
#include "ot-text.h"
#include <sstream>
#include <iomanip>

namespace ObTools { namespace Crypto {

//--------------------------------------------------------------------------
// Constructor
SHA1::SHA1(): finished(false)
{
  SHA1_Init(&sha_ctx);
}

//--------------------------------------------------------------------------
// Update digest with a block of data
void SHA1::update(const void *data, size_t length)
{
  if (!finished) SHA1_Update(&sha_ctx, data,
                             static_cast<unsigned long>(length));
}

//--------------------------------------------------------------------------
// Get result - writes DIGEST_LENGTH bytes to result
void SHA1::get_result(unsigned char *result)
{
  if (!finished) SHA1_Final(result, &sha_ctx);
  finished = true;
}

//--------------------------------------------------------------------------
// Get result as a hex string
string SHA1::get_result()
{
  if (!finished)
  {
    unsigned char buf[DIGEST_LENGTH];
    SHA1_Final(buf, &sha_ctx);
    finished = true;
    return Text::btox(buf, DIGEST_LENGTH);
  }
  else return "REUSED SHA1!";
}

//--------------------------------------------------------------------------
// Destructor
SHA1::~SHA1()
{
  if (!finished)
  {
    // Dump into a fake buffer
    unsigned char bucket[DIGEST_LENGTH];
    SHA1_Final(bucket, &sha_ctx);
  }
}

//--------------------------------------------------------------------------
// Static: Get hash of block of data.  Writes DIGEST_LENGTH bytes to result
void SHA1::digest(const void *data, size_t length, unsigned char *result)
{
  ::SHA1(static_cast<const unsigned char *>(data),
         static_cast<unsigned long>(length), result);
}

//------------------------------------------------------------------------
// Digest returning binary string
string SHA1::digest(const void *data, size_t length)
{
  unsigned char buf[DIGEST_LENGTH];
  digest(data, length, buf);
  return string(reinterpret_cast<char *>(buf), DIGEST_LENGTH);
}

//--------------------------------------------------------------------------
// Digest returning hex string
string SHA1::digest_hex(const void *data, size_t length)
{
  unsigned char buf[DIGEST_LENGTH];
  digest(data, length, buf);
  return Text::btox(buf, DIGEST_LENGTH);
}

}} // namespaces
