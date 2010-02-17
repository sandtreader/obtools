//==========================================================================
// ObTools::Text: test-convert.cc
//
// Test harness for text library numeric functions
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-text.h"
#include <iostream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main()
{
  string si = "1234567890";
  string sf = "12345.6789";
  string sx = "deadbeef";
  string si64 = "12345678901234567890";
  string sx64 = "fedcba9876543210";
  string sifix2 = "-0.89";
  string sifix2neg = "-12345678900";

  cout << "Integer string: " << si 
       << " -> " << Text::stoi(si) 
       << " -> " << Text::itos(Text::stoi(si)) << endl;

  cout << "Float string:   " << sf 
       << " -> " << Text::stof(sf) 
       << " -> " << Text::ftos(Text::stof(sf)) << endl;

  cout << "Hex string:     " << sx 
       << " -> " << hex << Text::xtoi(sx) << dec
       << " -> " << Text::itox(Text::xtoi(sx)) << endl;

  cout << "64-bit integer string: " << si64 
       << " -> " << Text::stoi64(si64) 
       << " -> " << Text::i64tos(Text::stoi64(si64)) << endl;

  cout << "64-bit hex string:     " << sx64 
       << " -> " << hex << Text::xtoi64(sx64) << dec 
       << " -> " << Text::i64tox(Text::xtoi64(sx64)) << endl;

  cout << "Integer representing fixed point to 2 d.p. string:  " << sifix2
       << " -> " << Text::stoifix(sifix2, 2)
       << " -> " << Text::ifixtos(Text::stoifix(sifix2, 2), 2) << endl;

  cout << "Integer representing fixed point to -2 d.p. string: " << sifix2neg
       << " -> " << Text::stoifix(sifix2neg, -2)
       << " -> " << Text::ifixtos(Text::stoifix(sifix2neg, -2), -2) << endl;

  unsigned char buf[4];
  int l = Text::xtob(sx, buf, 4);
  cout << "Hex string:     " << sx
       << " -> " << l << " bytes binary ["
       << hex << (int)buf[0] << (int)buf[1] << (int)buf[2] << (int)buf[3] 
       << dec << "] -> " << Text::btox(buf, 4) << endl;

  cout << "\nFloat formats for 1.0, 0.999999:\n";
  cout << "Default: " << Text::ftos(1.0) 
       << " " << Text::ftos(0.999999) << endl;
  cout << "(0,3): " << Text::ftos(1.0, 0, 3) 
       << " " << Text::ftos(0.999999, 0, 3) << endl;
  cout << "(16,6,true): " << Text::ftos(1.0, 16, 6, true) 
       << " " << Text::ftos(0.999999, 16, 6, true) << endl;

  return 0;  
}




