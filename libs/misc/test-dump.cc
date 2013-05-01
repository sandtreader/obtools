//==========================================================================
// ObTools::Text: test-dump.cc
//
// Test harness for Misc library random functions
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-misc.h"
#include <iostream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main()
{
  Misc::Dumper dumper(cout, 16, 4, true);

  string s("Mary had a little lamb, its fleece was white as snow\nAnd everywhere that Mary went, the lamb was sure to go\n");

  dumper.dump(s.c_str(), s.size());

  return 0;
}
