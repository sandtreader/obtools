//==========================================================================
// ObTools::MT: test-mutex.cc
//
// Test harness for mutex functions
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-mt.h"
#include <cstdlib>
#include <iostream>
#include <vector>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Mutex and global state
MT::Mutex mutex;
int n;  
int nback, nfore;

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
    nback++;
  }
}

//--------------------------------------------------------------------------
// Main
int main()
{
  n=0;
  nback=0;
  nfore=0;
  TestThread t;

  for(;;)
  {
    for(int i=0; i<100000; i++)
    {
      MT::Lock lock(mutex);
      n++;
      n--;
      nfore++;
    }

    cout << "N is " << n << " (" << nback << " in bg, " 
	 << nfore << " in fg)\n";
  }
  return 0;  
}




