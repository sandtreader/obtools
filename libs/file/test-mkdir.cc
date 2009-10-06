//==========================================================================
// ObTools::File: test-mkdir.cc
//
// Test harness for directory creation
//
// Copyright (c) 2009 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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
  if (argc < 2)
  {
    cout << "Give a directory name\n";
    return 2;
  }

  File::Directory dir(argv[1]);
  cout << "Directory: " << dir << endl;
  if (!dir.ensure(true))
  {
    cerr << "Can't create directory: " << strerror(errno) << endl;
    return 2;
  }

  return 0;  
}




