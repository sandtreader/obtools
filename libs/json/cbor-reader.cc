//==========================================================================
// ObTools::JSON: cbor-reader.cc
//
// CBOR encoder for JSON
//
// Copyright (c) 2024 Paul Clark.
//==========================================================================

#include "ot-json.h"

namespace ObTools { namespace JSON {

//------------------------------------------------------------------------
// Read an integer following the given first byte
uint64_t CBORReader::read_int(uint8_t initial_byte)
{
  auto ai = initial_byte & 0x1f;

  // Embedded in first byte
  if (ai < 24) return ai;

  // Variable length follows
  switch (ai)
  {
    case 24: return reader.read_byte();
    case 25: return reader.read_nbo_16();
    case 26: return reader.read_nbo_32();
    case 27: return reader.read_nbo_64();
    default:
      throw Channel::Error(11, "Unknown additional information "
                           + Text::itos(ai));
  }
}

//------------------------------------------------------------------------
// Read and decode a single CBOR value
Value CBORReader::decode()
{
  auto initial_byte = reader.read_byte();
  auto major_type = initial_byte >> 5;

  switch (major_type)
  {
    case 0:  // Positive integer
      return read_int(initial_byte);
      break;

    default:
      throw Channel::Error(10, "Unhandled major type "+Text::itos(major_type));
  }
}

}} // namespaces
