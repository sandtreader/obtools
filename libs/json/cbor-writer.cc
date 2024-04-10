//==========================================================================
// ObTools::JSON: cbor-writer.cc
//
// CBOR encoder for JSON
//
// Copyright (c) 2024 Paul Clark.
//==========================================================================

#include "ot-json.h"

namespace ObTools { namespace JSON {

//------------------------------------------------------------------------
// Output an integer with the given major type top 3 bits
void CBORWriter::write_int(uint64_t v, unsigned char major_type)
{
  auto first_byte = major_type << 5;

  if (v < 24)
  {
    writer.write_byte(first_byte | v);
  }
  else if (v < 256)
  {
    writer.write_byte(first_byte | 0x18);
    writer.write_byte(v);
  }
  else if (v < 65536)
  {
    writer.write_byte(first_byte | 0x19);
    writer.write_nbo_16(v);
  }
  else if (v < 0x100000000L)
  {
    writer.write_byte(first_byte | 0x1a);
    writer.write_nbo_32(v);
  }
  else
  {
    writer.write_byte(first_byte | 0x1b);
    writer.write_nbo_64(v);
  }
}

//------------------------------------------------------------------------
// Output a JSON value as CBOR
void CBORWriter::encode(const Value& v)
{
  switch (v.type)
  {
    case Value::INTEGER:
      if (v.n >= 0)
        write_int(v.n, 0);
      else
        write_int(-1-v.n, 1);
    break;

    case Value::FALSE_:
      writer.write_byte(0xf4);
    break;

    case Value::TRUE_:
      writer.write_byte(0xf5);
    break;

    case Value::NULL_:
      writer.write_byte(0xf6);
    break;

    case Value::UNSET:
      writer.write_byte(0xf7);
    break;

    case Value::BINARY:
      write_int(v.s.size(), 2);
      writer.write(v.s);
    break;

    case Value::STRING:
      write_int(v.s.size(), 3);
      writer.write(v.s);
    break;

    case Value::ARRAY:
      write_int(v.a.size(), 4);
      for(const auto& av: v.a)
        encode(av);
    break;

    case Value::OBJECT:
      write_int(v.o.size(), 5);
      for(const auto& it: v.o)
      {
        Value key(it.first);
        encode(key);
        encode(it.second);
      }
    break;

    default:;
  }
}

//------------------------------------------------------------------------
// Open an indefinite array
// Then continue to write any number of member values, and close it
void CBORWriter::open_indefinite_array()
{
  writer.write_byte(0x9f);
}

//------------------------------------------------------------------------
// Close an indefinite array
void CBORWriter::close_indefinite_array()
{
  writer.write_byte(0xff);
}

}} // namespaces
