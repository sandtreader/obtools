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
#define POLYNOMIAL 0xEDB88320L

//------------------------------------------------------------------------
// Constructor
CRC32::CRC32()
{
  // Generate combination table
  for(int i=0; i<256; i++)
  {
    crc_t crc;
    crc = i;
    for(int bit=0; bit<8; bit++)
    {
      if (crc & 1)
	crc = (crc >> 1) ^ POLYNOMIAL;
      else
	crc = crc >> 1;
    }
    combinations[i] = crc;
  }
}

//------------------------------------------------------------------------
// Calculate new CRC for a block
CRC32::crc_t CRC32::calculate(const unsigned char *data, size_t length)
{
  crc_t crc = 0xFFFFFFFFUL;  // All ones initialiser
    
  // Run each byte through table
  for(;length; length--)
  { 
    unsigned char byte = *data++;
    crc_t combiner = (crc & 0xff) ^ byte;
    crc = (crc >> 8) ^ combinations[combiner];     
  }

  return crc ^ 0xFFFFFFFFUL;  // Flip
}

//------------------------------------------------------------------------
// Calculate a CRC for a string (can be binary)
CRC32::crc_t CRC32::calculate(const string& data)
{
  return calculate((const unsigned char *)data.c_str(), data.size());
}


}} // namespaces
