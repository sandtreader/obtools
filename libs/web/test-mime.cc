//==========================================================================
// ObTools::Web:: test-mime.cc
//
// Test harness for MIME header functions
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
  ObTools::XML::Element root("MIME");
  ObTools::Web::MIMEHeaderParser mhp(root, cin);

  if (!mhp.parse())
  {
    cerr << "Parse failed\n";
    return 2;
  }

  cout << "\n--- XML form\n";
  cout << root;

  cout << "\n--- Regenerated\n";
  ObTools::Web::MIMEHeaderGenerator mhg(root, cout);
  if (!mhg.generate())
  {
    cerr << "Generate failed\n";
    return 2;
  }
  
  return 0;  
}




