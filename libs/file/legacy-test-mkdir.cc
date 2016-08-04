//==========================================================================
// ObTools::File: test-mkdir.cc
//
// Test harness for directory creation
//
// Copyright (c) 2009 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-file.h"
#include <iostream>
#include <errno.h>
#include <string.h>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  string p = "D:\\\350\257\267\350\276\223\345\205\245\345\205\263\351\224\256\345\255\227\\foo";

  if (argc >= 2) p = argv[1];
  File::Directory dir(p);
  cout << "Directory: " << dir << endl;
  if (!dir.ensure(true))
  {
    cerr << "Can't create directory: " << strerror(errno) << endl;
    return 2;
  }

  return 0;
}




