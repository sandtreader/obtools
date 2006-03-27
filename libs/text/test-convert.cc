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

  cout << "Integer string: " << si 
       << " -> " << Text::stoi(si) 
       << " -> " << Text::itos(Text::stoi(si)) << endl;

  cout << "Float string:   " << sf 
       << " -> " << Text::stof(sf) 
       << " -> " << Text::ftos(Text::stof(sf)) << endl;

  return 0;  
}




