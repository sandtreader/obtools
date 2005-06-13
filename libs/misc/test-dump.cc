//==========================================================================
// ObTools::Text: test-dump.cc
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
  Misc::Dumper dumper(cout);

  char *s = "Mary had a little lamb, its fleece was white as snow\nAnd everywhere that Mary went, the lamb was sure to go\n";

  dumper.dump(s, strlen(s)+1);

  return 0;  
}




