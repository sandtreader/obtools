//==========================================================================
// ObTools::Text: test-random.cc
//
// Test harness for Misc library random functions
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-misc.h"
#include <iostream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main()
{
  Misc::Random r;

  cout << "\nRandom number strings:\n";

  for(int i=1; i<=16; i++)
    cout << r.generate_hex(i) << endl;

  return 0;  
}




