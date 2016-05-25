//==========================================================================
// ObTools::File: test-dir.cc
//
// Test harness for directory iteration
//
// Copyright (c) 2005 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-file.h"
#include <iostream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  string p = "D:\\\350\257\267\350\276\223\345\205\245\345\205\263\351\224\256\345\255\227";
  if (argc >= 2) p = argv[1];
  File::Directory dir(p);
  cout << "Directory: " << dir << endl;

  list<File::Path> paths;
  if (!dir.inspect(paths))
  {
    cerr << "Can't inspect directory\n";
    return 2;
  }

  for(list<File::Path>::iterator p = paths.begin(); p!=paths.end(); ++p)
    cout << "  " << *p << endl;

  return 0;  
}




