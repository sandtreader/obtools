//==========================================================================
// ObTools::Text: test-case.cc
//
// Test harness for text library case functions
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
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
  string text = "RaD c00l dOOd!";

  cout << "Original: " << text << endl;
  cout << "Lower: " << Text::tolower(text) << endl;
  cout << "Upper: " << Text::toupper(text) << endl;

  return 0;  
}




