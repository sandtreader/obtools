//==========================================================================
// ObTools::Chan: bits.cc
//
// Bitstream reader/writer
//
// Copyright (c) 2007 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-chan.h"
#include "ot-net.h"

namespace ObTools { namespace Channel {

//==========================================================================
// Bitstream Reader

//--------------------------------------------------------------------------
// Read a single bit from the channel, returning an integer
// Throws SocketError on failure or EOF
int BitReader::read_bit() throw (Error)
{
  if (!bits_valid)
  {
    current_byte = reader.read_byte();
    bits_valid = 8;
  }

  return (current_byte >> --bits_valid) & 1;
}

//--------------------------------------------------------------------------
// Read up to 32 bits from the channel
// Returns bits in LSB of integer returned
// Throws SocketError on failure or EOF
uint32_t BitReader::read_bits(int n) throw (Error)
{
  uint32_t bits=0;
  while (n--)
  {
    if (!bits_valid)
    {
      current_byte = reader.read_byte();
      bits_valid = 8;
    }

    bits <<= 1;
    bits |= (current_byte >> --bits_valid) & 1;
  }

  return bits;
}

//==========================================================================
// Bitstream Writer

//--------------------------------------------------------------------------
// Write a single bit to the channel
// Throws SocketError on failure or EOF
void BitWriter::write_bit(int bit) throw (Error)
{
  // Add to current byte
  current_byte <<= 1;
  current_byte |= (bit&1);

  // Write out if we've reached 8 bits
  if (++bits_valid == 8)
  {
    writer.write_byte(current_byte);
    bits_valid = 0;
    current_byte = 0;
  }
}

//--------------------------------------------------------------------------
// Write up to 32 bits to the channel
// Writes bits from LSB of integer given
// Throws SocketError on failure or EOF
void BitWriter::write_bits(int n, uint32_t bits) throw (Error)
{
  while (n--)
  {
    // Accumulate bits into byte
    current_byte <<= 1;
    current_byte |= ((bits >> n) & 1);

    // Write out if we've reached 8 bits
    if (++bits_valid == 8)
    {
      writer.write_byte(current_byte);
      bits_valid = 0;
      current_byte = 0;
    }
  }
}

//--------------------------------------------------------------------------
// Flush remaining bits (if any) as a final byte, padding with zeros
// Throws SocketError on failure or EOF
void BitWriter::flush() throw (Error)
{
  if (bits_valid)
  {
    // Shift bits we have to top
    current_byte <<= (8-bits_valid);
    writer.write_byte(current_byte);

    // Clear down in case they carry on or call twice
    bits_valid = 0;
    current_byte = 0;
  }
}

}} // namespaces



