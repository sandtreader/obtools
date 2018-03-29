//==========================================================================
// ObTools::Text: uuid.cc
//
// UUID class
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-text.h"
#include <algorithm>

namespace ObTools { namespace Text {

//--------------------------------------------------------------------------
// Construction from a string
UUID::UUID(string str):
  array<byte, 16>{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}
{
  str.erase(remove(str.begin(), str.end(), '-'), str.end());
  xtob(str, &(*this)[0], size());
}

//--------------------------------------------------------------------------
// Get as string
string UUID::get_str() const
{
  auto str = get_hex_str();
  auto p = str.begin() + 2 * 4;
  str.insert(p, '-');
  p += 1 + 2 * 2;
  str.insert(p, '-');
  p += 1 + 2 * 2;
  str.insert(p, '-');
  p += 1 + 2 * 2;
  str.insert(p, '-');
  return str;
}

//--------------------------------------------------------------------------
// Get as plain hex string
string UUID::get_hex_str() const
{
  return btox(&(*this)[0], size());
}

//--------------------------------------------------------------------------
// Get as base64
string UUID::get_base64_str() const
{
  Text::Base64 base64;
  return base64.encode(&(*this)[0], size(), 0);
}

}} // namespaces
