//==========================================================================
// ObTools::Web:: test-url.cc
//
// Test harness for URL functions
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-web.h"
using namespace std;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  if (argc < 2) 
  {
    cout << "Supply a URL!\n";
    return 2;
  }

  cout << "\n---\nSplitting URL: " << argv[1] << endl;

  ObTools::XML::Element root("url");
  ObTools::Web::URLParser urlp(root);

  if (!urlp.parse(string(argv[1])))
  {
    cerr << "Parse failed\n";
    return 2;
  }

  cout << "\n--- XML form\n";
  cout << root;
 
  cout << "\n--- Regenerated\n";
  ObTools::Web::URLGenerator urlg(root);
  cout << urlg.generate() << endl;
 
  return 0;  
}




