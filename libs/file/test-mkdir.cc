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




