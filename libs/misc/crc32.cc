//==========================================================================
// ObTools::Misc: crc32.cc
//
// CRC32 implementation using byte-at-a-time combination table
// Algorithms adapted from example in PNG specification
//
// Copyright (c) 2007 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-misc.h"
#include <stdint.h>
#include <iomanip>

namespace ObTools { namespace Misc {

//------------------------------------------------------------------------
// Polynomials expressed as a bitmap with D<n> = x^n, top term left off
#define POLY_CRC32  ((1U<<26) + (1U<<23) + (1U<<22) + (1U<<16) + \
                     (1U<<12) + (1U<<11) + (1U<<10) + (1U<<8) + \
                     (1U<<7) + (1U<<5) + (1U<<4) + (1U<<2) + \
                     (1U<<1) + 1)
#define POLY_CRC32C ((1U<<28) + (1U<<27) + (1U<<26) + (1U<<25) + \
                     (1U<<23) + (1U<<22) + (1U<<20) + (1U<<19) + \
                     (1U<<18) + (1U<<14) + (1U<<13) + (1U<<11) + \
                     (1U<<10) + (1U<<9) + (1U<<8) + (1U<<6) + \
                     1)

// Reversed polynomials for use with LSB-first
#define POLY_CRC32_REV  ((1U<<31) + (1U<<30) + (1U<<29) + (1U<<27) + \
                         (1U<<26) + (1U<<24) + (1U<<23) + (1U<<21) + \
                         (1U<<20) + (1U<<19) + (1U<<15) + (1U<<9) + \
                         (1U<<8) + (1U<<5))
#define POLY_CRC32C_REV ((1U<<31) + (1U<<25) + (1U<<23) + (1U<<22) + \
                         (1U<<21) + (1U<<20) + (1U<<18) + (1U<<17) + \
                         (1U<<13) + (1U<<12) + (1U<<11) + (1U<<9) + \
                         (1U<<8) + (1U<<6) + (1U<<5) + (1U<<4) + \
                         (1U<<3))

//------------------------------------------------------------------------
// Constructor
CRC32::CRC32(Algorithm _alg, bool _reflected, bool _flip):
  algorithm(_alg), reflected(_reflected), flip(_flip)
{
  uint32_t poly;

  // Select polynomial
  if (reflected)
    poly = (algorithm == ALGORITHM_CRC32C ? POLY_CRC32C_REV : POLY_CRC32_REV);
  else
    poly = (algorithm == ALGORITHM_CRC32C ? POLY_CRC32C : POLY_CRC32);

  // Generate combination table
  for(int i=0; i<256; i++)
  {
    crc_t crc;
    if (reflected)
    {
      crc = i;
      for(int bit=0; bit<8; bit++)
      {
        if (crc & 1)
          crc = (crc >> 1) ^ poly;
        else
          crc = crc >> 1;
      }
    }
    else
    {
      crc = i << 24;
      for(int bit=0; bit<8; bit++)
      {
        if (crc & 0x80000000)
          crc = (crc << 1) ^ poly;
        else
          crc = crc << 1;
      }
    }
    combinations[i] = crc;
  }
}

//------------------------------------------------------------------------
// Calculate new CRC for a block
CRC32::crc_t CRC32::calculate(const unsigned char *data, size_t length)
{
  crc_t crc = initialiser();
  crc = consume(data, length, crc);
  return finalise(crc);
}

//------------------------------------------------------------------------
// Calculate a CRC for a string (can be binary)
CRC32::crc_t CRC32::calculate(const string& data)
{
  return calculate(reinterpret_cast<const unsigned char *>(data.c_str()),
                   data.size());
}

//--------------------------------------------------------------------------
// Stream-style usage

//--------------------------------------------------------------------------
// Get the initial value to work with
CRC32::crc_t CRC32::initialiser() const
{
  switch (algorithm)
  {
    default:
      return 0xFFFFFFFFUL;  // All ones initialiser
  }
}

//--------------------------------------------------------------------------
// Consume some data and update CRC
CRC32::crc_t CRC32::consume(const unsigned char *data, size_t length,
                            crc_t crc) const
{
  // Run each byte through table
  if (reflected)
  {
    for(;length; length--)
    {
      unsigned char byte = *data++;
      crc_t combiner = (crc & 0xff) ^ byte;
      crc = (crc >> 8) ^ combinations[combiner];
    }
  }
  else
  {
    for(;length; length--)
    {
      unsigned char byte = *data++;
      crc_t combiner = (crc >> 24) ^ byte;
      crc = ((crc << 8) & 0xffffffff) ^ combinations[combiner];
    }
  }
  return crc;
}

//--------------------------------------------------------------------------
// Finalise CRC
CRC32::crc_t CRC32::finalise(crc_t crc) const
{
  if (flip) crc ^= 0xFFFFFFFF;
  return crc;
}

}} // namespaces
