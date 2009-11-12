//==========================================================================
// ObTools::Time: test-duration.cc
//
// Test harness for time library - reading and converting durations 
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-time.h"
#include <iostream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main()
{
  // Show clock
  cout << "Monotonic clock: " << Time::Duration::clock().seconds() << endl;

  // Read a line at a time, and convert to duration
  while (!cin.eof())
  {
    string line;
    getline(cin, line);

    Time::Duration d(line);

    cout << line << " -> " 
	 << d.seconds() << "s, NTP: " << d.ntp()
	 << ", HMS: " << d.hms() << endl;
  }

  return 0;
}




