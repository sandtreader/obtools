//==========================================================================
// ObTools::Misc: crc.cc
//
// CRC16 implementation using byte-at-a-time combination table
// Algorithms adapted from ancient (now dead) 'sysmag' code by Paul Clark
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-misc.h"
#include <stdint.h>
#include <iomanip>

namespace ObTools { namespace Misc {

//------------------------------------------------------------------------
// Polynomials expressed as a bitmap with D<n> = x^n, top term left off 
#define CRC_CCITT ((1U<<12) + (1<<5) + 1)
#define CRC_16    ((1U<<15) + (1<<2) + 1)

// Reversed polynomials for use with LSB-first 
#define CRC_CCITT_REV ((1U<<15) + (1<<10) + (1<<3))
#define CRC_16_REV    ((1U<<15) + (1<<13) + 1)

//------------------------------------------------------------------------
// Constructor
CRC::CRC(Algorithm _alg, bool _reflected, bool _flip): 
  algorithm(_alg), reflected(_reflected), flip(_flip)
{
  int poly;

  // Select polynomial
  if (reflected)
    poly = (algorithm==ALGORITHM_CRC16)?CRC_16_REV:CRC_CCITT_REV;
  else
    poly = (algorithm==ALGORITHM_CRC16)?CRC_16:CRC_CCITT;

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
      crc = i << 8;
      for(int bit=0; bit<8; bit++)
      {
	if (crc & 0x8000)
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
CRC::crc_t CRC::calculate(const unsigned char *data, size_t length)
{
  crc_t crc;

  // Select starting point
  switch (algorithm)
  {
    case ALGORITHM_CCITT:
      crc = 0xFFFF;
      break;

    case ALGORITHM_CCITT_ZERO:
    case ALGORITHM_CRC16:
    default:
      crc = 0;
      break;

    case ALGORITHM_CCITT_MOD:
      crc = reflected?0xF0B8:0x1D0F;  // Joe Geluso's theory, plus reversal
      break;
  }
    
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
      crc_t combiner = (crc>>8) ^ byte;
      crc = ((crc << 8)&0xffff) ^ combinations[combiner];
    }
  }

  if (flip) crc ^= 0xFFFF;

  return crc;
}

//------------------------------------------------------------------------
// Calculate a CRC for a string (can be binary)
CRC::crc_t CRC::calculate(const string& data)
{
  return calculate((const unsigned char *)data.c_str(), data.size());
}

}} // namespaces
