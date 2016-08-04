//==========================================================================
// ObTools::Time: test-duration.cc
//
// Test harness for time library - reading and converting durations
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-time.h"
#include "ot-text.h"
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
  int rc = 0;
  while (!cin.eof())
  {
    string line;
    getline(cin, line);
    if (line.empty()) continue;
    if (line[0] == '#') continue;

    const vector<string> bits = Text::split(line, '|');
    if (bits.size() != 2)
    {
      cerr << "Bad line [" << line << "]\n";
      rc = 2;
      continue;
    }

    const Time::Duration d(bits[0]);
    const double secs = Text::stof(bits[1]);

    cout << bits[0] << " -> "
         << d.seconds() << "s, NTP: " << d.ntp()
         << ", HMS: " << d.hms() << endl;

    // Use stringification to provide reasonable rounding
    if (Text::ftos(d.seconds()) != Text::ftos(secs))
    {
      cerr << "Expected " << secs << " got " << d.seconds() << endl;
      rc = 2;
    }
  }

  return rc;
}




