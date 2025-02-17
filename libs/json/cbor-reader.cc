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

    case 4:  // Array
    {
      Value array(Value::ARRAY);
      if (initial_byte == 0x9f)
      {
        // Indefinite
        for(;;)
        {
          auto v = decode();
          if (v.type == Value::BREAK) break;
          array.a.push_back(v);
        }
      }
      else
      {
        // Definite
        auto len = read_int(initial_byte);
        for(auto i=0u; i<len; i++)
          array.a.push_back(decode());
      }

      return array;
    }

    case 5:  // Object
    {
      Value object(Value::OBJECT);
      if (initial_byte == 0xbf)
      {
        // Indefinite
        for(;;)
        {
          auto key = decode();
          if (key.type == Value::BREAK) break;
          if (key.type == Value::STRING)
            object.o[key.s] = decode();
          else if (key.type == Value::INTEGER)
            object.o[Text::itos(key.n)] = decode();
          else
            throw Channel::Error(13,
                       "Can't handle non-string or integer CBOR object keys");
        }
      }
      else
      {
        // Definite
        auto len = read_int(initial_byte);
        for(auto i=0u; i<len; i++)
        {
          auto key = decode();
          if (key.type == Value::STRING)
            object.o[key.s] = decode();
          else if (key.type == Value::INTEGER)
            object.o[Text::itos(key.n)] = decode();
          else
            throw Channel::Error(13,
                       "Can't handle non-string or integer CBOR object keys");
        }
      }
      return object;
    }

    case 6: // Semantic tags
      {
        auto type = initial_byte & 0x1f;
        if (type > 23) type = reader.read_byte();
        switch (type)
        {
          case 24: // embedded CBOR, for deferred decoding
            {
              // Grossly inefficient implementation
              // Ideally would defer any decoding
              const auto str = decode().cbor();
              auto bin = vector<byte>(str.size());
              transform(str.begin(), str.end(), bin.begin(),
                  [] (const char c) {
                    return reinterpret_cast<const byte &>(c);
                  });
              return Value{bin};
            }
          default:
            throw Channel::Error(14, "Unhandled tag type " + Text::itos(type));
        }
      }

    case 7:  // Floats & simple
      switch (initial_byte & 0x1f)
      {
        case 20: return Value(Value::FALSE_);
        case 21: return Value(Value::TRUE_);
        case 22: return Value(Value::NULL_);
        case 23: return Value();
        case 31: return Value(Value::BREAK);
        default: throw Channel::Error(12, "Unhandled float/simple type "
                                      +Text::itos(initial_byte & 0x1f));
      }

    default:
      throw Channel::Error(10, "Unhandled major type "+Text::itos(major_type));
  }
}

//------------------------------------------------------------------------
// Read the open of an indefinite array
// Then continue to read any number of member values until you get BREAK
// Returns whether the first byte is an indefinite array (0x9f)
// Consumes a byte if it isn't - rewind the reader if you need to handle
// definite arrays as well
bool CBORReader::open_indefinite_array()
{
  return reader.read_byte() == 0x9f;
}

}} // namespaces
