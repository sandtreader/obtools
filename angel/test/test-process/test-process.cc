//==========================================================================
// ObTools::Angel:test: test-process.cc
//
// Simple test process to exercise angeld
//
// Copyright (c) 2005 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include <iostream>
#include <unistd.h>

using namespace std;

//--------------------------------------------------------------------------
// Main

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    cout << "Angel test process:\n";
    cout << "  test-process <name> [<time to run>]\n";
    return 0;
  }

  char *name = argv[1];
  int runtime = 0;
  if (argc > 2) runtime=atoi(argv[2]);

  cout << "This is test process '" << name << "'\n";
  cerr << name << ": test error\n";

  if (runtime)
    sleep(runtime);
  else
    pause();

  cout << "Test process '" << name << "' exiting\n";

  return 0;  
}




