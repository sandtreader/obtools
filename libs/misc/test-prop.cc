//==========================================================================
// ObTools::Misc: test-prop.cc
//
// Test harness for propertylist functions
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
  Misc::PropertyList pl;
  pl.add("foo", "one");
  pl.add("bar", "!!!");
  pl.add("bar", "two");
  pl.add("XXX", "???");
  pl.add("42", 42);
  pl.add_bool("true", true);

  pl.erase("XXX");

  cout << " 'foo' is '" << pl["foo"] << "'\n";
  cout << " 'XXX' is '" << pl["XXX"] << "'\n";

  pl.dump(cout, "-- ");

  // Read lines to interpolate from stdin
  while (cin)
  {
    string line;
    cin >> line;
    cout << "[" << line << "] -> [" << pl.interpolate(line) << "]\n";
  }

  return 0;  
}




