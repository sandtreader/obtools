//==========================================================================
// ObTools::Text: test-md5.cc
//
// Test harness for text library md5 functions
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-text.h"
#include <iostream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main()
{
  string text = "Top secret message";

  string md5 = Text::md5(text);

  cout << "MD5 of '" << text << "' is " << md5 << endl;

  return 0;  
}




