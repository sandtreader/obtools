//==========================================================================
// ObTools::Web:: test-http-file.cc
//
// Test harness for HTTP message functions
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-web.h"
using namespace std;

//--------------------------------------------------------------------------
// Main

int main()
{
  ObTools::XML::Element root("HTTP");
  ObTools::Web::HTTPMessageParser hmp(root, cin);

  if (!hmp.parse())
  {
    cerr << "Parse failed\n";
    return 2;
  }

  cout << "\n--- XML form\n";
  cout << root;
 
  cout << "\n--- Regenerated\n";
  ObTools::Web::HTTPMessageGenerator hmg(root, cout);
  if (!hmg.generate())
  {
    cerr << "Generate failed\n";
    return 2;
  }
  
 
  return 0;  
}




