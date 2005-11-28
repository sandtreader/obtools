//==========================================================================
// ObTools::File: test-dir.cc
//
// Test harness for directory iteration
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-file.h"
#include <iostream>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cout << "Give a directory name\n";
    return 2;
  }

  File::Directory dir(argv[1]);
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




