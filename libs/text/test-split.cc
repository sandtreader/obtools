//==========================================================================
// ObTools::Text: test-split.cc
//
// Test harness for text library split functions
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
  string text = "foo, bar,  splat wibble,,wombat ";

  cout << "Original: " << text << endl;
  vector<string> l = Text::split(text);
  cout << "Default split:\n";
  for(vector<string>::iterator p = l.begin(); p!=l.end(); ++p)
    cout << "  [" << *p << "]\n";

  return 0;  
}




