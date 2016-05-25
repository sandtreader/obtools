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
  Time::DateStamp now = Time::Stamp::now();
  time_t t = now.time();
  cout << "now -> " << asctime(gmtime(&t))
       << " -> " << now.iso() << " (" << days[now.weekday()-1] << ")"
       << endl;

  // Read a line at a time, and convert to stamp
  while (!cin.eof())
  {
    string line;
    getline(cin, line);

    Time::DateStamp s(line);

    time_t t = s.time();

    cout << "--------------------" << endl;
    cout << "        Input: " << line << endl;
    cout << "     ISO date: " << s.iso() << endl;
    cout << "   check time: " << asctime(gmtime(&t));
    cout << "      weekday: " << days[s.weekday()-1] << endl;

    Time::DateStamp local = s.localise();
    cout << "    localised: " << local.iso() << endl;
    cout << "   globalised: " << local.globalise().iso() << endl;

    cout << "  locale date: " << s.locale_date() << endl;
  }

  return 0;
}
