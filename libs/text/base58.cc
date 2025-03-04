//==========================================================================
// ObTools::Text: base58.cc
//
// Base58 encoding/decoding
//
// Copyright (c) 2024 Paul Clark.  All rights reserved
//==========================================================================

#include "ot-text.h"
#include <ctype.h>
#include <cstring>

namespace ObTools { namespace Text {

// Encoding alphabet from draft-msporny-base58-03
static const char base58_chars[] =
 "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

//------------------------------------------------------------------------
// Constructor with standard alphabet
Base58::Base58()
{
  memcpy(map, base58_chars, sizeof(map));
  reverse_map.fill(-1);
  for (int i=0; i<58; ++i)
    reverse_map[static_cast<int>(base58_chars[i])] = i;
}

//------------------------------------------------------------------------
// Constructor with custom alphabet (58 characters)
Base58::Base58(const char *alphabet)
{
  memcpy(map, alphabet, sizeof(map));
  reverse_map.fill(-1);
  for (int i=0; i<58; ++i)
    reverse_map[static_cast<int>(map[i])] = i;
}

//--------------------------------------------------------------------------
// Encode a binary vector - options as encode above
string Base58::encode(const vector<byte>& binary)
{
  if (binary.empty())  return "";

  // Count leading zeros
  int zeroes = 0;
  for (auto b: binary)
  {
    if (!(int)b)
      ++zeroes;
    else
      break;
  }

  // Allocate enough space in big-endian base58 representation
  // log(256) / log(58), rounded up
  int size = (binary.size() - zeroes) * 138 / 100 + 1;
  vector<int> b58(size);

  // Process the bytes
  // This is the algorithm from the bitcoin code, via ChatGPT
  // It seems spectacularly inefficient to have two nested loops here!
  for (auto byte: binary)
  {
    int carry = (int)byte;
    for (auto it = b58.rbegin(); it != b58.rend(); ++it)
    {
      carry += 256 * (*it);
      *it = carry % 58;
      carry /= 58;
    }
  }

  // Skip leading zeros in base58 result
  auto it = b58.begin();
  while (it != b58.end() && *it == 0) ++it;

  // Translate the result into a string
  string result;
  result.reserve(zeroes + (b58.end() - it));
  result.assign(zeroes, '1');
  while (it != b58.end())
    result += map[*(it++)];

  return result;
}

//--------------------------------------------------------------------------
// Decode base64 text into a binary buffer
// Returns whether successful - if so, appends data to binary
bool Base58::decode(const string& base58, vector<byte>& binary)
{
  // Decode base58 string to big integer
  // size log(58) / log(256), rounded up
  std::vector<int> b256((base58.size() * 733) / 1000 + 1);
  for (char c: base58)
  {
    int value = reverse_map[static_cast<unsigned char>(c)];
    if (value == -1) return false;  // Invalid

    int carry = value;
    for (auto it = b256.rbegin(); it != b256.rend(); ++it)
    {
      carry += 58 * (*it);
      *it = carry % 256;
      carry /= 256;
    }
  }

  // Skip leading zeros in b256 result
  auto it = b256.begin();
  while (it != b256.end() && !*it) ++it;

  // Append result to binary
  binary.clear();
  binary.reserve(std::distance(it, b256.end()));
  while (it != b256.end())
  {
    binary.push_back(static_cast<byte>(*it));
    ++it;
  }

  // Restore leading zeros from base58 input
  int leading_zeros = 0;
  for (char c: base58)
  {
    if (c == map[0])
      ++leading_zeros;
    else
      break;
  }

  binary.insert(binary.begin(), leading_zeros, byte{0});
  return true;
}

}} // namespaces
