//==========================================================================
// ObTools::Crypto: sha1.cc
//
// SHA1 digest/hash
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include <stdlib.h>
#include "ot-crypto.h"
#include <sstream>
#include <iomanip>

namespace OpenSSL
{
}

namespace ObTools { namespace Crypto {

//------------------------------------------------------------------------
// Constructor
SHA1::SHA1(): finished(false)
{
  OpenSSL::SHA1_Init(&sha_ctx);
}

//------------------------------------------------------------------------
// Update digest with a block of data
void SHA1::update(const void *data, size_t length)
{
  if (!finished) OpenSSL::SHA1_Update(&sha_ctx, data, (unsigned long)length);
}

//------------------------------------------------------------------------
// Get result - writes DIGEST_LENGTH bytes to result
void SHA1::get_result(unsigned char *result)
{
  if (!finished) OpenSSL::SHA1_Final(result, &sha_ctx);
  finished = true;
}

//------------------------------------------------------------------------
// Get 20-byte hex string
string SHA1::hex20(unsigned char *b)
{
  ostringstream oss;
  oss << hex << setw(2) << setfill('0');
  for(int i=0; i<DIGEST_LENGTH; i++, b++) oss << (int)*b;
  return oss.str();
}

//------------------------------------------------------------------------
// Get result as a hex string
string SHA1::get_result()
{
  if (!finished)
  {
    unsigned char buf[DIGEST_LENGTH];
    OpenSSL::SHA1_Final(buf, &sha_ctx);
    finished = true;
    return hex20(buf);
  }
  else return "REUSED SHA1!";
}

//------------------------------------------------------------------------
// Destructor
SHA1::~SHA1()
{
  if (!finished)
  {
    // Dump into a fake buffer
    unsigned char bucket[DIGEST_LENGTH];
    OpenSSL::SHA1_Final(bucket, &sha_ctx);
  }
}

//------------------------------------------------------------------------
// Static: Get hash of block of data.  Writes DIGEST_LENGTH bytes to result
void SHA1::digest(const void *data, size_t length, unsigned char *result)
{
  OpenSSL::SHA1((const unsigned char *)data, (unsigned long)length, result);
}

//------------------------------------------------------------------------
// Ditto, but returning hex string
string SHA1::digest(const void *data, size_t length)
{
  unsigned char buf[DIGEST_LENGTH];
  digest(data, length, buf);
  return hex20(buf);
}

}} // namespaces
