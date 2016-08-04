//==========================================================================
// ObTools::Text: test-case.cc
//
// Test harness for text library case functions
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
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
  string text = "RaD c00l dOOd!";

  cout << "Original: " << text << endl;
  cout << "Lower: " << Text::tolower(text) << endl;
  cout << "Upper: " << Text::toupper(text) << endl;

  return 0;
}




