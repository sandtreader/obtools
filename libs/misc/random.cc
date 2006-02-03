//==========================================================================
// ObTools::Misc: random.cc
//
// Randomiser function - generate random strings based on best random
// input available
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-misc.h"
#include <sstream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>

namespace ObTools { namespace Misc {

//------------------------------------------------------------------------
// Constructor
Random::Random()
{
  // Do this even in Unix just in case /dev/urandom fails
  srand(time(NULL));
}

//------------------------------------------------------------------------
// Get random binary bytes up to N bytes long
void Random::generate_binary(unsigned char *p, int n)
{
#if !defined(__WIN32__)
  // In Real Operating Systems (tm), /dev/urandom gives us an unlimited 
  // supply of random bytes from a hardware-assisted entropy pool
  ifstream f("/dev/urandom", ios_base::binary);  

  if (f)
  {
    f.read((char *)p, n);

    // Check we read it, otherwise drop through to pseudo
    if (f.gcount() == n) return;
  }
#endif

  // Generate pseudo-random as a fallback
  for(int i=0; i<n; i++) p[i] = (unsigned char)(rand() & 0xff);
}

//------------------------------------------------------------------------
// Get a random hex string up to N bytes long
string Random::generate_hex(int n)
{
  unsigned char *p = new unsigned char[n];
  generate_binary(p, n);

  ostringstream oss;
  for(int i=0; i<n; i++)
    oss << hex << setfill('0') << setw(2) << (unsigned)p[i];

  delete[] p;
  return oss.str();
}

//------------------------------------------------------------------------
// Get a random 32-bit number
uint32_t Random::generate_32()
{
  unsigned char buf[4];
  generate_binary(buf, 4);
  return *(uint32_t *)buf;  // Big endian, little endian, who cares?
}

//------------------------------------------------------------------------
// Get a random 64-bit number
uint64_t Random::generate_64()
{
  unsigned char buf[8];
  generate_binary(buf, 8);
  return *(uint64_t *)buf;  
}

//------------------------------------------------------------------------
// Get a random number in the range 0 and (n-1)
unsigned int Random::generate_up_to(unsigned int n)
{
  return generate_32() % n;
}

}} // namespaces
