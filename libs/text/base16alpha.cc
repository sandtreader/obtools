//==========================================================================
// ObTools::Text: alpha16.cc
//
// Base16 encoding/decoding into an alpha-only character set believed safe
// from 'bad' words 
//
// Alphabet is "bcdg hjkl mpqr svwz" - no vowels, and also missing "fntxy"
//
// Copyright (c) 2007 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-text.h"
#include <ctype.h>

namespace ObTools { namespace Text {

static const char alphabet[] = "bcdghjklmpqrsvwz";

// Reverse array of alpha, -1 is disallowed
static const int reverse[] = 
  { -1, 0, 1, 2, -1, -1, 3, 
    4, -1, 5, 6, 7,
    8, -1, -1, 9, 10, 11, 
    12, -1, -1, 13, 14, -1, -1, 15 };

//--------------------------------------------------------------------------
// Encode a 64-bit integer 
// Uses as many characters as required
string Base16Alpha::encode(uint64_t n)
{
  if (!n) return "b"; // Special case

  string base16;

  while (n)
  {
    int r = (int)(n % 16);
    base16.insert(base16.begin(), alphabet[r]);  // Prepend
    n/=16;
  }

  return base16;
}

//--------------------------------------------------------------------------
// Decode a 64-bit integer
// Returns whether successful - if so, sets 'n'
bool Base16Alpha::decode(const string& base16, uint64_t& n)
{
  if (base16.empty()) return false;

  n = 0;
  for(string::const_iterator p = base16.begin(); p!=base16.end(); ++p)
  {
    char c = *p;
    int cn;

    // Check ranges
    if (c>='A' && c<='Z')
      cn = c-'A';
    else if (c>='a' && c<='z')
      cn = c-'a';
    else
      return false;  // Fail on anything else

    int dn = reverse[cn];
    if (dn<0) return false;

    n = n*16+dn;
  }

  return true;
}


}} // namespaces
