//==========================================================================
// ObTools::Text: test-range.cc
//
// Test harness for Misc library RangeSet
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
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
  Misc::UInt64RangeSet s(5);
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
      Misc::UInt64RangeSet::offset_t start, length;
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
    cout << "[" << s.gauge(50) << "]" << " (" << s.total_length << ")\n";
    cout << "Complete? " << (s.is_complete()?"Yes":"No") << endl;
    cout << "Coverage: " << s.coverage() << ", " 
	 << s.percentage_complete() << "%\n";
    cout << "Includes 15,5? " << (s.contains(15,5)?"Yes":"No") << endl;

    // Set operations
    Misc::UInt64RangeSet so;
    so.insert(0,10);
    so.insert(50,10);

    cout << "+{0,10;50,10}:\n";
    cout << (s+so);
    cout << "-{0,10;50,10}:\n";
    cout << (s-so);
    cout << "^{0,10;50,10}:\n";
    cout << (s^so);

    // Test string output/input
    cout << "Text:\n";
    string text = s.str();
    cout << text << endl;
    Misc::UInt64RangeSet s1(text);
    cout << "Read back from text:\n";
    cout << s1 << endl;

    // Test XML
    cout << "XML:\n";
    XML::Element e("rangeset");
    s.add_to_xml(e);
    cout << e;
    Misc::UInt64RangeSet s2;
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

    Misc::UInt64RangeSet s3;
    Channel::BlockReader br(buf, bw.get_offset());
    s3.read(br);
    cout << "Read back from binary (total length "<< s3.total_length<< "):\n";
    cout << s3 << endl;

    // Invert
    Misc::UInt64RangeSet inv = s.inverse();
    cout << "Inverse:\n";
    cout << inv;
    // Gauge
    cout << "[" << inv.gauge(50) << "]"  << " (" << inv.total_length << ")\n";

    cout << "\n-----------------------------------------------------------------------------\n\n";

  }

  return 0;  
}




