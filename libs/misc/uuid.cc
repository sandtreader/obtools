//==========================================================================
// ObTools::Misc: uuid.cc
//
// UUID class
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-text.h"
#include "ot-misc.h"
#include <algorithm>

namespace ObTools { namespace Misc {

//--------------------------------------------------------------------------
// Construction from a string
UUID::UUID(string str):
  array<byte, 16>{{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}}
{
  str.erase(remove(str.begin(), str.end(), '-'), str.end());
  Text::xtob(str, &(*this)[0], size());
}

//--------------------------------------------------------------------------
// Get as UUID style string
string UUID::get_str() const
{
  auto str = get_hex_str();
  str.insert(20, "-");
  str.insert(16, "-");
  str.insert(12, "-");
  str.insert(8, "-");
  return str;
}

//--------------------------------------------------------------------------
// Get as plain hex string
string UUID::get_hex_str() const
{
  return Text::btox(&(*this)[0], size());
}

//--------------------------------------------------------------------------
// Get as base64
string UUID::get_base64_str() const
{
  Text::Base64 base64;
  return base64.encode(&(*this)[0], size(), 0);
}

//--------------------------------------------------------------------------
// Set to a random value
void UUID::randomise()
{
  Misc::Random random;
  do
  {
    random.generate_binary(&(*this)[0], size());
  }
  while (operator!());
}

//--------------------------------------------------------------------------
// Test has a value
bool UUID::operator!() const
{
  for (auto i = 0u; i < size(); ++i)
    if ((*this)[i])
      return false;
  return true;
}

//--------------------------------------------------------------------------
// << operator to write UUID to ostream
ostream& operator<<(ostream& s, const UUID& uuid)
{
  s << uuid.get_str();
  return s;
}

}} // namespaces
