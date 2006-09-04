//==========================================================================
// ObTools::Text: test-range.cc
//
// Test harness for Misc library RangeSet
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
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
  Misc::RangeSet s;

  // Read numbers from cin
  while (cin)
  {
    Misc::RangeSet::off_t start, length;
    cin >> start >> length;
    if (!cin) break;

    cout << "Inserting " << length << " at " << start << endl;
    s.insert(start, length);
    cout << s;
    cout << "[" << s.gauge() << "]" << endl;
  }

  return 0;  
}




