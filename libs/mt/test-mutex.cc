//==========================================================================
// ObTools::MT: test-mutex.cc
//
// Test harness for mutex functions
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-mt.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <unistd.h>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Mutex and global state
MT::Mutex mutex;
int n;  

//--------------------------------------------------------------------------
// Test thread class
class TestThread: public MT::Thread
{
  virtual void run();

public:
  TestThread() { start(); }
};

void TestThread::run()
{
  for(;;)
  {
    MT::Lock lock(mutex);
    n++;
    n--;
  }
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  n=0;
  TestThread t;

  for(;;)
  {
    for(int i=0; i<100000; i++)
    {
      MT::Lock lock(mutex);
      n++;
      n--;
    }

    cout << "N is " << n << endl;
  }
  return 0;  
}




