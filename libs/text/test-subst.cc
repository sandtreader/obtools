//==========================================================================
// ObTools::Text: test-subst.cc
//
// Test harness for text library substitution functions
//
// Copyright (c) 2004 xMill Consulting Limited.  All rights reserved
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
  string text = "foo bar foo fo";

  cout << "Original: " << text << endl;
  if (Text::subst(text, "foo", "fool"))
    cout << "s/foo/fool/g: " << text << endl;
  else
    return 2;

  return 0;  
}




