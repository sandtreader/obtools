//==========================================================================
// ObTools::Text: test-md5.cc
//
// Test harness for text library md5 functions
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-misc.h"
#include <iostream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main()
{
  string text = "Top secret message";
  Misc::MD5 md5er;
  string md5 = md5er.sum(text);

  cout << "MD5 of '" << text << "' is " << md5 << endl;

  return 0;  
}




