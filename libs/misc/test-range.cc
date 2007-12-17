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
  Misc::RangeSet s(5);
  bool begin = true;

  // Read numbers from cin
  while (cin)
  {
    if (begin)
    {
      cout << "At start, empty:\n";
      begin = false;
    }
    else
    {
      Misc::RangeSet::off_t start, length;
      char action;

      cin >> action >> start >> length;
      if (!cin) break;

      switch (action)
      {
	case '+':
	  cout << "Inserting " << length << " at " << start << endl;
	  s.insert(start, length);
	  break;

	case '-':
	  cout << "Removing " << length << " at " << start << endl;
	  s.remove(start, length);
	  break;

	default:
	  cout << "Bogus action '" << action << "' ignored\n";
	  break;
      }
    }

    cout << s;
    cout << "[" << s.gauge(s.total_length<50?s.total_length:50) << "]" 
	 << " (" << s.total_length << ")\n";
    cout << "Complete? " << (s.is_complete()?"Yes":"No") << endl;
    cout << "Coverage: " << s.coverage() << ", " 
	 << s.percentage_complete() << "%\n";
    cout << "Includes 15,5? " << (s.contains(15,5)?"Yes":"No") << endl;

    // Test XML
    cout << "XML:\n";
    XML::Element e("rangeset");
    s.add_to_xml(e);
    cout << e;
    Misc::RangeSet s2;
    s2.read_from_xml(e);
    cout << "Read back from XML (total length " << s2.total_length << "):\n";
    cout << s2 << endl;

    // Test binary
    cout << "Binary:\n";
    unsigned char buf[8000];
    Channel::BlockWriter bw(buf, sizeof(buf));
    s.write(bw);

    Misc::Dumper dumper(cout);
    dumper.dump(buf, bw.get_offset());

    Misc::RangeSet s3;
    Channel::BlockReader br(buf, bw.get_offset());
    s3.read(br);
    cout << "Read back from binary (total length "<< s3.total_length<< "):\n";
    cout << s3 << endl;

    // Invert
    Misc::RangeSet inv = s.invert();
    cout << "Inverse:\n";
    cout << inv;
    cout << "[" << inv.gauge(inv.total_length<50?inv.total_length:50) << "]" 
	 << " (" << inv.total_length << ")\n";

    cout << "\n-----------------------------------------------------------------------------\n\n";
  }

  return 0;  
}




