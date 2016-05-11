//==========================================================================
// ObTools::Huffman: value.cc
//
// Huffman Value implementation
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-huffman.h"

namespace ObTools { namespace Huffman {

ostream& operator<<(ostream& os, const Value& value)
{
  if (value.is_special())
  {
    switch (value.get_special_value())
    {
      case Value::Special::none:
        os << "NONE";
        break;
      case Value::Special::start:
        os << "START";
        break;
      case Value::Special::stop:
        os << "STOP";
        break;
      case Value::Special::escape:
        os << "ESCAPE";
        break;
    }
  }
  else
  {
    os << value.get_value();
  }
  return os;
}

}} // namespaces
