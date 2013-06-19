//==========================================================================
// ObTools::Crypto: pkcs5.cc
//
// PKCS5 padding
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <stdlib.h>
#include "ot-crypto.h"

namespace ObTools { namespace Crypto {

//------------------------------------------------------------------------
// Pad a block of data to given length multiple
// Returns copied and padded malloc'ed data block, and modifies length
// to length of padded block
unsigned char *PKCS5::pad(const unsigned char *data, int& length, 
			  int multiple)
{
  // Always adds padding, even if not really needed
  int new_length = (length+multiple)/multiple*multiple;
  int extra = new_length-length;
  unsigned char *new_data = static_cast<unsigned char *>(malloc(new_length));
  if (!new_data) abort();

  // Copy data and add padding
  memcpy(new_data, data, length);
  memset(new_data+length, extra, extra);
  
  length = new_length;
  return new_data;
}

//------------------------------------------------------------------------
// Pad a block of data to given length multiple, in place
// Data must be at least multiple bytes longer than length
// Returns length of padded data
int PKCS5::pad_in_place(unsigned char *data, int length, int multiple)
{
  // Always adds padding, even if not really needed
  int new_length = (length+multiple)/multiple*multiple;
  int extra = new_length-length;

  // Add padding
  memset(data+length, extra, extra);

  return new_length;
}

//------------------------------------------------------------------------
// Unpad a block of data 
// Returns original length of block - data is not copied or modified
int PKCS5::original_length(const unsigned char *data, int length)
{
  if (!length) return 0;
  int pad = data[length-1];
  if (pad > length) pad=0;  // Broken or not padded, leave padding in
  return length-pad;
}


}} // namespaces
