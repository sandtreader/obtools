//==========================================================================
// ObTools::Misc: random.cc
//
// Randomiser function - generate random strings based on best random
// input available
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-misc.h"
#include <sstream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#include <time.h>
#else
#include <sys/time.h>
#define USE_DEV_URANDOM 1
#endif

#define REINIT_PERIOD 67

namespace ObTools { namespace Misc {

//--------------------------------------------------------------------------
// Constructor
Random::Random(): w(0), z(0), count(0)
{}

//--------------------------------------------------------------------------
// Get random binary bytes up to N bytes long
void Random::generate_binary(unsigned char *p, int n)
{
  // Reinitialise every REINIT_PERIOD calls, including first time
  if (!(count++ % REINIT_PERIOD))
  {
#ifdef USE_DEV_URANDOM
    // In Real Operating Systems (tm), /dev/urandom gives us an unlimited
    // supply of random bytes from a hardware-assisted entropy pool
    // but it's slow, so don't do it every time, just use it to reset the
    // PRNG now and again
    ifstream f("/dev/urandom", ios_base::binary);

    if (f)
    {
      // Reseed from entropy bytes
      f.read(reinterpret_cast<char *>(&w), 4);
      f.read(reinterpret_cast<char *>(&z), 4);
    }
    else
#endif

    // Initialise from time — only reached when /dev/urandom unavailable
    {  // LCOV_EXCL_START
#if defined(PLATFORM_WINDOWS)
      FILETIME ft;
      GetSystemTimeAsFileTime(&ft);
      w ^= ft.dwHighDateTime^ft.dwLowDateTime;
      z ^= ft.dwLowDateTime;
#else
      struct timeval tv;
      gettimeofday(&tv, 0);
      w ^= tv.tv_sec ^ tv.tv_usec;
      z ^= tv.tv_usec;
#endif
    }  // LCOV_EXCL_STOP
  }

  // Generate pseudo-random as a fallback
  // Marsaglia Multiply-With-Carry (MWC), converted to produce bytes
  for(int i=0; i<n; i++)
  {
    z = 36969 * (z & 65535) + (z >> 16);
    w = 18000 * (w & 65535) + (w >> 16);

    // Use all the bits to make a byte
    p[i] = static_cast<unsigned char>(z^w^(z>>8)^(w>>8)^(z>>16)^(w>>16)
                                      ^(z>>24)^(w>>24));
  }
}

//--------------------------------------------------------------------------
// Generate 'n' bytes of binary into a vector<byte>
vector<byte> Random::generate_binary(int n)
{
  vector<byte> buf(n);
  generate_binary(reinterpret_cast<unsigned char *>(buf.data()), n);
  return buf;
}

//--------------------------------------------------------------------------
// Get a random hex string up to N bytes long
string Random::generate_hex(int n)
{
  unsigned char *p = new unsigned char[n];
  generate_binary(p, n);

  ostringstream oss;
  for(int i=0; i<n; i++)
    oss << hex << setfill('0') << setw(2) << static_cast<unsigned>(p[i]);

  delete[] p;
  return oss.str();
}

//--------------------------------------------------------------------------
// Get a random 32-bit number
uint32_t Random::generate_32()
{
  uint32_t n; // Big endian, little endian, who cares?
  generate_binary(reinterpret_cast<unsigned char *>(&n), 4);
  return n;
}

//--------------------------------------------------------------------------
// Get a random 64-bit number
uint64_t Random::generate_64()
{
  uint64_t n;
  generate_binary(reinterpret_cast<unsigned char *>(&n), 8);
  return n;
}

//--------------------------------------------------------------------------
// Get a random number in the range 0 and (n-1)
unsigned int Random::generate_up_to(unsigned int n)
{
  return generate_32() % n;
}

}} // namespaces
