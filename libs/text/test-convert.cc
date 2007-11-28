//==========================================================================
// ObTools::Text: test-convert.cc
//
// Test harness for text library numeric functions
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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

  cout << "Integer string: " << si 
       << " -> " << Text::stoi(si) 
       << " -> " << Text::itos(Text::stoi(si)) << endl;

  cout << "Float string:   " << sf 
       << " -> " << Text::stof(sf) 
       << " -> " << Text::ftos(Text::stof(sf), 16, 6, true) << endl;

  cout << "Hex string:     " << sx 
       << " -> " << hex << Text::xtoi(sx) << dec
       << " -> " << Text::itox(Text::xtoi(sx)) << endl;

  cout << "64-bit integer string: " << si64 
       << " -> " << Text::stoi64(si64) 
       << " -> " << Text::i64tos(Text::stoi64(si64)) << endl;

  cout << "64-bit hex string:     " << sx64 
       << " -> " << hex << Text::xtoi64(sx64) << dec 
       << " -> " << Text::i64tox(Text::xtoi64(sx64)) << endl;

  unsigned char buf[4];
  int l = Text::xtob(sx, buf, 4);
  cout << "Hex string:     " << sx
       << " -> " << l << " bytes binary ["
       << hex << (int)buf[0] << (int)buf[1] << (int)buf[2] << (int)buf[3] 
       << dec << "] -> " << Text::btox(buf, 4) << endl;

  return 0;  
}




