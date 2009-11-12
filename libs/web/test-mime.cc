//==========================================================================
// ObTools::Web:: test-mime.cc
//
// Test harness for MIME header functions
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-web.h"
using namespace std;

//--------------------------------------------------------------------------
// Main

int main()
{
  ObTools::Web::MIMEHeaders headers;

  if (!headers.read(cin))
  {
    cerr << "Parse failed\n";
    return 2;
  }

  cout << "\n--- XML form\n";
  cout << headers.xml;

  cout << "\n--- Foo headers, split at commas:\n";
  list<string> foos = headers.get_all("foo");
  for(list<string>::iterator p=foos.begin();
      p!=foos.end();
      p++)
    cout << "Foo: " << *p << endl;

  cout << "\n--- Split header, split:\n";
  string s = headers.get("split");
  ObTools::Misc::PropertyList pl = headers.split_parameters(s);
  cout << "First: " << s << endl;
  cout << "Parameters:\n" << pl; 

  cout << "\n--- Regenerated\n";
  cout << headers;
  
  return 0;  
}




