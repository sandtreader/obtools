//==========================================================================
// ObTools::Misc: test-prop.cc
//
// Test harness for propertylist functions
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#include "ot-misc.h"
#include <iostream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main()
{
  Misc::PropertyList pl;
  pl.add("foo", "one");
  pl.add("bar", "!!!");
  pl.add("bar", "two");
  pl.add("XXX", "???");
  pl.erase("XXX");

  cout << " 'foo' is '" << pl["foo"] << "'\n";
  cout << " 'XXX' is '" << pl["XXX"] << "'\n";

  pl.dump(cout, "-- ");

  return 0;  
}




