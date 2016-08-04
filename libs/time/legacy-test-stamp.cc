//==========================================================================
// ObTools::Time: test-stamp.cc
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

const char *days[] = { "Monday", "Tuesday", "Wednesday", "Thursday",
		       "Friday", "Saturday", "Sunday" };

//--------------------------------------------------------------------------
// Main
int main()
{
  Time::Stamp now = Time::Stamp::now();
  time_t t = now.time();
  cout << "now -> " << asctime(gmtime(&t))
       << " -> " << now.iso() << " (" << days[now.weekday()-1] << ")\n";

  // Read a line at a time, and convert to stamp
  while (!cin.eof())
  {
    string line;
    getline(cin, line);

    Time::Stamp s(line);

    time_t t = s.time();

    cout << "--------------------\n";
    cout << "        Input: " << line << endl;
    cout << "          ISO: " << s.iso() << endl;
    cout << "   check time: " << asctime(gmtime(&t));
    cout << "      weekday: " << days[s.weekday()-1] << endl;
    cout << "      ISO min: " << s.iso_minimal() << endl;
    cout << "      ISO num: " << s.iso_numeric() << endl;
    cout << "     ISO date: " << s.iso_date() << endl;
    cout << "     ISO time: " << s.iso_time(':', true) << endl;
    cout << "       RFC822: " << s.rfc822() << endl;

    Time::Stamp local = s.localise();
    cout << "    localised: " << local.iso() << endl;
    cout << "   globalised: " << local.globalise().iso() << endl;

    cout << "  locale date: " << s.locale_date() << endl;
    cout << "  locale time: " << s.locale_time() << endl;
    cout << "  locale both: " << s.locale_date_time() << endl;
    cout << "    formatted: " << s.format("%H:%M %A, %-e %B") << endl;
  }

  return 0;
}




