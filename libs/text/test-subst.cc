//==========================================================================
// ObTools::Text: test-subst.cc
//
// Test harness for text library substitution functions
//
// Copyright (c) 2004 Paul Clark.  All rights reserved
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
  string text = "foo bar foo fo";

  cout << "Original: " << text << endl;
  text = Text::subst(text, "foo", "fool");
  cout << "s/foo/fool/g: " << text << endl;

  return 0;  
}




