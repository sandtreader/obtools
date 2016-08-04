//==========================================================================
// ObTools::Web:: test-http-file.cc
//
// Test harness for HTTP message functions
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
  ObTools::Web::HTTPMessage msg;

  if (!msg.read(cin))
  {
    cerr << "Parse failed\n";
    return 2;
  }

  if (msg.is_request())
  {
    cout << msg.version << " request: " << msg.method << " for "
         << msg.url << endl;
  }
  else
  {
    cout << msg.version << " response: " << msg.code << " - "
         << msg.reason << endl;
  }

  cout << msg.headers.xml;
  if (msg.body.size()) cout << "Body:\n" << msg.body << endl;

  cout << "\n--- Regenerated\n";
  cout << msg << endl;

  return 0;
}




