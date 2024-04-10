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
      return Value(read_int(initial_byte));

    case 1:  // Negative integer
      return Value(-1-read_int(initial_byte));

    case 2:  // Binary
    {
      auto len = read_int(initial_byte);
      Value value(Value::BINARY);
      reader.read(value.s, len);
      return value;
    }

    case 3:  // String
    {
      auto len = read_int(initial_byte);
      Value value(Value::STRING);
      reader.read(value.s, len);
      return value;
    }

    case 7:  // Floats & simple
      switch (initial_byte & 0x1f)
      {
        case 20: return Value(Value::FALSE_);
        case 21: return Value(Value::TRUE_);
        case 22: return Value(Value::NULL_);
        case 23: return Value();
        default: throw Channel::Error(12, "Unhandled float/simple type "
                                      +Text::itos(initial_byte & 0x1f));
      }

    default:
      throw Channel::Error(10, "Unhandled major type "+Text::itos(major_type));
  }
}

}} // namespaces
