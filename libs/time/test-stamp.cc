//==========================================================================
// ObTools::Time: test-stamp.cc
//
// Test harness for time library - reading and converting durations 
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-time.h"
#include <iostream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main
int main()
{
  Time::Stamp now = Time::Stamp::now();
  time_t t = now.time();
  cout << "now -> " << asctime(gmtime(&t))
       << " -> " << now.iso() << endl;

  // Read a line at a time, and convert to stamp
  while (!cin.eof())
  {
    string line;
    getline(cin, line);

    Time::Stamp s(line);

    time_t t = s.time();
    
    cout << line << " -> " << s.iso()
	 << " -> " << asctime(gmtime(&t)) << endl;
  }

  return 0;
}




