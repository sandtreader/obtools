//==========================================================================
// ObTools::Text: test-md5.cc
//
// Test harness for Misc library md5 functions
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-misc.h"
#include <iostream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  string text = "Top secret message";
  if (argc > 1) text = argv[1];

  Misc::MD5 md5er;
  string md5 = md5er.sum(text);

  cout << "MD5 of '" << text << "' is " << md5 
       << " (" << md5.size() << " characters)\n";

  return 0;  
}




