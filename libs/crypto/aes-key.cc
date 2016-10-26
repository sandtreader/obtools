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
#include "ot-text.h"

namespace ObTools { namespace Crypto {

//--------------------------------------------------------------------------
// Destructor
// Trashes the key!
AESKey::~AESKey()
{
  // Manual trashing in case memset in compromised
  for (unsigned int i = 0; i < sizeof(key); ++i)
  {
    key[i] = 0xff;
    key[i] = 0;
  }
}

//--------------------------------------------------------------------------
// Create a new key from random data
// Must seed PRNG first
void AESKey::create()
{
  if (!RAND_bytes(&key[0], size / 8))
    RAND_pseudo_bytes(&key[0], size / 8);  // Fallback
  valid = true;
}

//--------------------------------------------------------------------------
// Read from data block
void AESKey::read(const unsigned char *data)
{
  memcpy(key, data, size / 8);
  valid = true;
}

//--------------------------------------------------------------------------
// Write to data block - writes 8 bytes of data
void AESKey::write(unsigned char *data) const
{
  memcpy(data, key, size / 8);
}

//--------------------------------------------------------------------------
// Read from stream - reads hex characters
void AESKey::read(istream& sin)
{
  memset(&key, 0, sizeof(key));  // In case string is too short
  for (int i = 0; i < size / 4; ++i)
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

//--------------------------------------------------------------------------
// Read from stream as binary
void AESKey::read_binary(istream& sin)
{
  sin.read(reinterpret_cast<char *>(key), size/8);
  valid = sin.gcount() == size/8;
}


//--------------------------------------------------------------------------
// Write to stream - write hex characters
void AESKey::write(ostream& sout) const
{
  for (int i = 0; i < size / 8; ++i)
    sout << hex << setw(2) << setfill('0') << static_cast<int>(key[i]);
  sout << dec << setw(0) << setfill(' ');
}

//--------------------------------------------------------------------------
// Write to stream as binary
void AESKey::write_binary(ostream& sout) const
{
  sout.write(reinterpret_cast<const char *>(key), size/8);
}

//--------------------------------------------------------------------------
// Read from string - reads hex characters
void AESKey::read(const string& text)
{
  istringstream iss(text);
  read(iss);
}

//------------------------------------------------------------------------
// Read from string as binary
void AESKey::read_binary(const string& binary)
{
  istringstream iss(binary);
  read_binary(iss);
}

//--------------------------------------------------------------------------
// Set from passphrase
void AESKey::set_from_passphrase(const string& text)
{
  // Make sure we generate enough for the longest key
  // 20*2 = 40 > 32 bytes (256 bits)
  unsigned char sha1[SHA1::DIGEST_LENGTH*2];

  // SHA1 the text into the first half
  SHA1::digest(text.data(), text.size(), sha1);

  // SHA1 the first half into the second
  SHA1::digest(sha1, SHA1::DIGEST_LENGTH, sha1+SHA1::DIGEST_LENGTH);

  // Read from the hash
  read(sha1);
}

//--------------------------------------------------------------------------
// Set from integer - big-endian, zero padded
void AESKey::set_from_int(uint64_t n)
{
  Channel::BlockWriter writer(key, size/8);
  try
  {
    writer.skip(size/8-8);
    writer.write_nbo_64(n);
    valid = true;
  }
  catch (const Channel::Error&)
  {}
}


//--------------------------------------------------------------------------
// Convert to string
string AESKey::str() const
{
  ostringstream oss;
  write(oss);
  return oss.str();
}

//--------------------------------------------------------------------------
// Read from channel (binary bytes)
void AESKey::read(Channel::Reader& reader) throw (Channel::Error)
{
  reader.read(key, size / 8);
}

//--------------------------------------------------------------------------
// Write to channel (binary bytes)
void AESKey::write(Channel::Writer& writer) const throw (Channel::Error)
{
  writer.write(key, size / 8);
}

//--------------------------------------------------------------------------
// Read from string as base64
// Returns whether successful
bool AESKey::set_from_base64(const string& s)
{
  Text::Base64 base64;
  if (static_cast<int>(base64.decode(s, key, size/8)) != size/8) return false;
  valid = true;
  return true;
}

//--------------------------------------------------------------------------
// Convert to a base64 string
string AESKey::str_base64() const
{
  Text::Base64 base64;
  return base64.encode(key, size/8);
}

//--------------------------------------------------------------------------
// >> operator to read key from istream
istream& operator>>(istream& s, AESKey& k)
{
  k.read(s);
  return s;
}

//--------------------------------------------------------------------------
// << operator to write key to ostream
ostream& operator<<(ostream& s, const AESKey& k)
{
  k.write(s);
  return s;
}

}} // namespaces
