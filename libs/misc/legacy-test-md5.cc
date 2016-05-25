//==========================================================================
// ObTools::Misc: test-md5.cc
//
// Test harness for Misc library md5 functions
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
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




