//==========================================================================
// ObTools::Text: base36.cc
//
// Base36 encoding/decoding (numbers only)
//
// Copyright (c) 2007 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-text.h"
#include <ctype.h>

namespace ObTools { namespace Text {

static const char alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyz";

//--------------------------------------------------------------------------
// Encode a 64-bit integer 
// Uses as many characters as required
string Base36::encode(uint64_t n)
{
  if (!n) return "0"; // Special case

  string base36;

  while (n)
  {
    int r = static_cast<int>(n % 36);
    base36.insert(base36.begin(), alphabet[r]);  // Prepend
    n/=36;
  }

  return base36;
}

//--------------------------------------------------------------------------
// Decode a 64-bit integer
// Returns whether successful - if so, sets 'n'
bool Base36::decode(const string& base36, uint64_t& n)
{
  if (base36.empty()) return false;

  n = 0;
  for(string::const_iterator p = base36.begin(); p!=base36.end(); ++p)
  {
    char c = *p;
    int cn;

    // Check ranges
    if (c>='A' && c<='Z')
      cn = 10+c-'A';
    else if (c>='a' && c<='z')
      cn = 10+c-'a';
    else if (c>='0' && c<='9')
      cn = c-'0';
    else
      return false;  // Fail on anything else

    n = (n*36) + cn;
  }

  return true;
}


}} // namespaces
