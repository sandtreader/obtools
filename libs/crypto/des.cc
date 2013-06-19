//==========================================================================
// ObTools::Crypto: des.cc
//
// DES encryption/decryption
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <stdlib.h>
#include "ot-crypto.h"
#include <openssl/opensslv.h>

// Make build with both 0.9.7 and 0.9.8
#if OPENSSL_VERSION_NUMBER >= 0x908000L
#define DES_CBLOCK_CAST(_x) ((DES_cblock *)(_x))
#else
#define DES_CBLOCK_CAST(_x) _x
#endif

namespace ObTools { namespace Crypto {

//------------------------------------------------------------------------
// Encrypt/decrypt a block in place
// If block is not padded to 8 bytes, the remainder (up to 7) bytes 
// WILL NOT BE ENCRYPTED
// Encrypts if 'encryption' is set (default), otherwise decrypts
// IV is modified if set
// Returns whether successful (keys set up correctly)
#pragma GCC diagnostic ignored "-Wold-style-cast"
bool DES::encrypt(unsigned char *data, int length, bool encryption)
{
  // Round length down to 8-byte multiple
  length = 8*(length/8);

  // Choose encryption or decryption
  int enc = encryption?DES_ENCRYPT:DES_DECRYPT;

  // Check for CBC - IV is valid
  if (iv.valid)
  {
    // CBC
    // These routines can do the whole thing in one

    switch (nkeys)
    {
      case 1:
	DES_ncbc_encrypt(data, data, length, &keys[0].schedule, &iv.key, enc);
	break;

      case 2:
	DES_ede2_cbc_encrypt(data, data, length, &keys[0].schedule, 
			     &keys[1].schedule, &iv.key, enc);
	break;

      case 3:
	DES_ede3_cbc_encrypt(data, data, length,
			     &keys[0].schedule, &keys[1].schedule,
			     &keys[2].schedule, &iv.key, enc);
	break;

      default: return false;
    }
  }
  else
  {
    // ECB
    for(int i=0; i<length; i+=8, data+=8)
    {
      switch (nkeys)
      {
	case 1:
	{
          DES_cblock *block = reinterpret_cast<DES_cblock *>(data);
	  DES_ecb_encrypt(block, block, &keys[0].schedule, enc);
	}
	break;

	case 2:
	  DES_ecb2_encrypt(DES_CBLOCK_CAST(data), 
			   DES_CBLOCK_CAST(data), 
			   &keys[0].schedule, &keys[1].schedule,
			   enc);
	  break;

	case 3:
	  DES_ecb3_encrypt(DES_CBLOCK_CAST(data), 
			   DES_CBLOCK_CAST(data), 
			   &keys[0].schedule, &keys[1].schedule,
			   &keys[2].schedule, enc);
	  break;

	default: return false;
      }
    }
  }

  return true;
}
#pragma GCC diagnostic pop

}} // namespaces




