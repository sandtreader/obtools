//==========================================================================
// ObTools::Text: test-case.cc
//
// Test harness for text library case functions
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-text.h"
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




