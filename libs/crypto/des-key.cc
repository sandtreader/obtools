//==========================================================================
// ObTools::Crypto: des-key.cc
//
// DES key handling
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include <ctype.h>
#include "ot-chan.h"
#include "ot-crypto.h"

namespace ObTools { namespace Crypto {

//------------------------------------------------------------------------
// Copy constructor - just copies base key and rebuilds schedule
DESKey::DESKey(const DESKey& k):
  is_key(k.is_key)
{
  memcpy(key, k.key, 8);
  load();
}

//------------------------------------------------------------------------
// Assignment operator - ditto
DESKey& DESKey::operator=(const DESKey& k)
{
  is_key=k.is_key;
  memcpy(key, k.key, 8);
  load();
  return *this;
}

//------------------------------------------------------------------------
// Check and load the key from base data - sets valid true or false 
// after checking it
void DESKey::load()
{
  // IVs are always valid, and schedule not used
  valid = is_key?!DES_set_key_checked(&key, &schedule):true;
}

//------------------------------------------------------------------------
// Create a new key from random data
// Must seed PRNG first 
void DESKey::create()
{
  do
  {
    DES_random_key(&key);
    if (is_key) DES_set_odd_parity(&key);
  } while (is_key && DES_is_weak_key(&key));

  load();
}

//------------------------------------------------------------------------
// Read from data block - requires 8 bytes of data 
void DESKey::read(const unsigned char *data)
{
  memcpy(key, data, 8);
  load();
}

//------------------------------------------------------------------------
// Write to data block - writes 8 bytes of data 
void DESKey::write(unsigned char *data) const
{
  memcpy(data, key, 8);
}

//------------------------------------------------------------------------
// Read from stream - reads 16 hex characters
void DESKey::read(istream& sin)
{
  int i;
  
  for(i=0; i<16; i++)
  {
    int n = sin.get();
    if (n<0) break;

    // Read hex nybble
    n = toupper(n);
    n = (n>='A')?(n-'A'+10):(n-'0');

    // Use hex nybble
    if (i%2)
      key[i/2] += n;
    else
      key[i/2] = n*16;
  }

  valid = false;
  if (i==16) load();
}

//------------------------------------------------------------------------
// Write to stream - write 16 hex characters
void DESKey::write(ostream& sout) const
{
  for(int i=0; i<8; i++)
    sout << hex << setw(2) << setfill('0') << (int)key[i];
  sout << dec << setw(0) << setfill(' ');
}

//------------------------------------------------------------------------
// Read from string - reads 16 hex characters
void DESKey::read(const string& text)
{
  istringstream iss(text);
  read(iss);
}

//------------------------------------------------------------------------
// Convert to string
string DESKey::str() const
{
  ostringstream oss;
  write(oss);
  return oss.str();
}
  
//------------------------------------------------------------------------
// Read from channel (8 binary bytes)
void DESKey::read(Channel::Reader& reader) throw (Channel::Error)
{
  reader.read(key, 8);
  load();
}

//------------------------------------------------------------------------
// Write to channel (8 binary bytes)
void DESKey::write(Channel::Writer& writer) const throw (Channel::Error)
{
  writer.write(key, 8);
}

//------------------------------------------------------------------------
// >> operator to read key from istream
istream& operator>>(istream& s, DESKey& k)
{
  k.read(s);
  return s;
}

//------------------------------------------------------------------------
// << operator to write key to ostream
ostream& operator<<(ostream& s, const DESKey& k)
{
  k.write(s);
  return s;
}

}} // namespaces

