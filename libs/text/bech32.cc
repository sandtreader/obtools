//==========================================================================
// ObTools::Text: bech32.cc
//
// Base58 encoding/decoding
//
// Copyright (c) 2024 Paul Clark.  All rights reserved
//==========================================================================

#include "ot-text.h"
#include "ot-gen.h"
#include <ctype.h>

namespace ObTools { namespace Text {

// Encoding alphabet from BIP173
static constexpr array<pair<uint8_t, char>, 32> data{{
  {0, 'q'},
  {1, 'p'},
  {2, 'z'},
  {3, 'r'},
  {4, 'y'},
  {5, '9'},
  {6, 'x'},
  {7, '8'},
  {8, 'g'},
  {9, 'f'},
  {10, '2'},
  {11, 't'},
  {12, 'v'},
  {13, 'd'},
  {14, 'w'},
  {15, '0'},
  {16, 's'},
  {17, '3'},
  {18, 'j'},
  {19, 'n'},
  {20, '5'},
  {21, '4'},
  {22, 'k'},
  {23, 'h'},
  {24, 'c'},
  {25, 'e'},
  {26, '6'},
  {27, 'm'},
  {28, 'u'},
  {29, 'a'},
  {30, '7'},
  {31, 'l'},
}};
static constexpr Gen::ConstExprMap<uint8_t, char, 32> map{data};

//--------------------------------------------------------------------------
// Encode a binary vector
string Bech32::encode(const vector<byte>& binary)
{
  const auto numbits = binary.size() * 8;
  auto bitpos = size_t{0};
  auto result = string{};
  while (bitpos < numbits)
  {
    uint8_t num = 0x1F &
        (Gen::shiftr(static_cast<uint8_t>(binary[bitpos / 8]),  3 - bitpos % 8) +
         ((bitpos % 8) > 3 && ((1 + bitpos / 8) < binary.size())
          ? static_cast<uint8_t>(binary[1 + (bitpos / 8)]) >> (11 - bitpos % 8)
          : 0));
    result.push_back(map.lookup(num));
    bitpos += 5;
  }
  return result;
}

//--------------------------------------------------------------------------
// Decode bech32 text into a binary buffer
// Returns whether successful - if so, appends data to binary
bool Bech32::decode(const string& bech32, vector<byte>& binary)
{
  try
  {
    auto bytepos = uint8_t{0};
    auto b = uint8_t{0};
    for (const auto c: bech32)
    {
      auto num = map.reverse_lookup(c);
      b |= Gen::shiftl(num, 8 - (bytepos + 5));
      if (bytepos + 5 >= 8)
      {
        binary.push_back(static_cast<byte>(b));
        b = num << (8 - ((bytepos + 5) % 8));
      }
      bytepos = (bytepos + 5) % 8;
    }
    return true;
  }
  catch (const range_error&)
  {
    return false;
  }
}

//--------------------------------------------------------------------------
// Decode bech32 text into a binary buffer as 5-bit data
// Returns whether successful - if so, appends data to binary
bool Bech32::decode_as_5_bit(const string& bech32, vector<uint8_t>& binary)
{
  try
  {
    for (const auto c: bech32)
    {
      auto num = map.reverse_lookup(c);
      binary.push_back(num);
    }
    return true;
  }
  catch (const range_error&)
  {
    return false;
  }
}

}} // namespaces
