//==========================================================================
// ObTools::Crypto: aes-key.cc
//
// AES key handling
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include <ctype.h>
#include <openssl/rand.h>
#include "ot-chan.h"
#include "ot-crypto.h"

namespace ObTools { namespace Crypto {

//------------------------------------------------------------------------
// Create a new key from random data
// Must seed PRNG first
void AESKey::create()
{
  RAND_bytes(&key[0], size / 8);
  valid = true;
}

//------------------------------------------------------------------------
// Read from data block
void AESKey::read(const unsigned char *data)
{
  memcpy(key, data, size / 8);
  valid = true;
}

//------------------------------------------------------------------------
// Write to data block - writes 8 bytes of data
void AESKey::write(unsigned char *data) const
{
  memcpy(data, key, size / 8);
}

//------------------------------------------------------------------------
// Read from stream - reads hex characters
void AESKey::read(istream& sin)
{
  int i;

  for (i = 0; i < size / 4; ++i)
  {
    int n = sin.get();
    if (n < 0) break;

    // Read hex nybble
    n = toupper(n);
    n = (n >= 'A') ? (n - 'A' + 10) : (n - '0');

    // Use hex nybble
    if (i % 2)
      key[i / 2] += n;
    else
      key[i / 2] = n * 16;
  }
  valid = true;
}

//------------------------------------------------------------------------
// Write to stream - write hex characters
void AESKey::write(ostream& sout) const
{
  for (int i = 0; i < size / 8; ++i)
    sout << hex << setw(2) << setfill('0') << (int)key[i];
  sout << dec << setw(0) << setfill(' ');
}

//------------------------------------------------------------------------
// Read from string - reads hex characters
void AESKey::read(const string& text)
{
  istringstream iss(text);
  read(iss);
}

//------------------------------------------------------------------------
// Convert to string
string AESKey::str() const
{
  ostringstream oss;
  write(oss);
  return oss.str();
}

//------------------------------------------------------------------------
// Read from channel (binary bytes)
void AESKey::read(Channel::Reader& reader) throw (Channel::Error)
{
  reader.read(key, size / 8);
}

//------------------------------------------------------------------------
// Write to channel (binary bytes)
void AESKey::write(Channel::Writer& writer) const throw (Channel::Error)
{
  writer.write(key, size / 8);
}

//------------------------------------------------------------------------
// >> operator to read key from istream
istream& operator>>(istream& s, AESKey& k)
{
  k.read(s);
  return s;
}

//------------------------------------------------------------------------
// << operator to write key to ostream
ostream& operator<<(ostream& s, const AESKey& k)
{
  k.write(s);
  return s;
}

}} // namespaces