//==========================================================================
// ObTools::JSON: cbor.cc
//
// CBOR encoder for JSON
//
// Copyright (c) 2024 Paul Clark.
//==========================================================================

#include "ot-json.h"

namespace ObTools { namespace JSON {

//------------------------------------------------------------------------
// Output an integer with the given major type top 3 bits
void CBOR::write_cbor_int_to(Channel::Writer& w, uint64_t v,
                             unsigned char major_type)
{
  auto first_byte = major_type << 5;

  if (v < 24)
    w.write_byte(first_byte | v);
  else if (v < 256)
  {
    w.write_byte(first_byte | 0x18);
    w.write_byte(v);
  }
  else if (v < 65536)
  {
    w.write_byte(first_byte | 0x19);
    w.write_nbo_16(v);
  }
  else if (v < 0x100000000L)
  {
    w.write_byte(first_byte | 0x1a);
    w.write_nbo_32(v);
  }
  else
  {
    w.write_byte(first_byte | 0x1b);
    w.write_nbo_64(v);
  }
}

//------------------------------------------------------------------------
// Output a JSON value as CBOR to the given channel
void CBOR::encode(const Value& v, Channel::Writer& w)
{
  switch (v.type)
  {
    case Value::INTEGER:
      if (v.n >= 0)
        write_cbor_int_to(w, v.n, 0);
      else
        write_cbor_int_to(w, -1-v.n, 1);
    break;

    case Value::FALSE_:
      w.write_byte(0xf4);
    break;

    case Value::TRUE_:
      w.write_byte(0xf5);
    break;

    case Value::NULL_:
      w.write_byte(0xf6);
    break;

    case Value::UNSET:
      w.write_byte(0xf7);
    break;

    case Value::BINARY:
      write_cbor_int_to(w, v.s.size(), 2);
      w.write(v.s);
    break;

    case Value::STRING:
      write_cbor_int_to(w, v.s.size(), 3);
      w.write(v.s);
    break;

    case Value::ARRAY:
      write_cbor_int_to(w, v.a.size(), 4);
      for(const auto& av: v.a)
        encode(av, w);
    break;

    case Value::OBJECT:
      write_cbor_int_to(w, v.o.size(), 5);
      for(const auto& it: v.o)
      {
        Value key(it.first);
        key.write_cbor_to(w);
        encode(it.second, w);
      }
    break;

    default:;
  }
}

}} // namespaces
