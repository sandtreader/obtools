//==========================================================================
// ObTools::Crypto: ot-crypto.h
//
// Public definitions for ObTools::Crypto
// C++ wrappers for crypto functions, (currently) using OpenSSL
// 
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_CRYPTO_H
#define __OBTOOLS_CRYPTO_H

#include <string>
#include <map>

#include <openssl/des.h>

namespace ObTools { namespace Crypto { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// DES key (des.cc)
class DESKey
{
private:
  void load();

public:
  DES_cblock key;                 // Original 8-byte key
  DES_key_schedule schedule;      // Expanded key for optimised processing
  bool valid;                     // Whether key/schedule is valid
  
  //------------------------------------------------------------------------
  // Default constructor
  DESKey(): valid(false) {}

  //------------------------------------------------------------------------
  // Constructor from data block - requires 8 bytes of data
  DESKey(const unsigned char *data) { read(data); }

  //------------------------------------------------------------------------
  // Constructor from string - requires 16 hex characters
  DESKey(const string& text) { read(text); }

  //------------------------------------------------------------------------
  // Create a new key from random data
  // Must seed PRNG first 
  void create();

  //------------------------------------------------------------------------
  // Read from data block - requires 8 bytes of data 
  void read(const unsigned char *data);

  //------------------------------------------------------------------------
  // Write to data block - writes 8 bytes of data 
  void write(unsigned char *data) const;

  //------------------------------------------------------------------------
  // Read from stream - reads 16 hex characters
  void read(istream& sin);

  //------------------------------------------------------------------------
  // Write to stream - reads 16 hex characters
  void write(ostream& sout) const;

  //------------------------------------------------------------------------
  // Read from string - reads 16 hex characters
  void read(const string& text);

  //------------------------------------------------------------------------
  // Convert to string
  string str() const;
};

//------------------------------------------------------------------------
// >> operator to read key from istream
istream& operator>>(istream& s, DESKey& k);

//------------------------------------------------------------------------
// << operator to write key to ostream
ostream& operator<<(ostream& s, const DESKey& k);

//==========================================================================
// PKCS5 padding support 
class PKCS5
{
 public:
  //------------------------------------------------------------------------
  // Pad a block of data to given length multiple
  // Returns copied and padded malloc'ed data block, and modifies length
  // to length of padded block
  static unsigned char *pad(const unsigned char *data, int& length, 
			    int multiple);

  //------------------------------------------------------------------------
  // Unpad a block of data 
  // Returns original length of block - data is not copied or modified
  static int PKCS5::original_length(const unsigned char *data, int length);
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_CRYPTO_H
















