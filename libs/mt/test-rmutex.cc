//==========================================================================
// ObTools::MT: test-rmutex.cc
//
// Test harness for recursive mutex functions
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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
MT::RMutex mutex;
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
    {
      MT::RLock lock(mutex);
      {
	// Lock it again
	MT::RLock lock2(mutex);
	n++;
	n--;
	nback++;
      }
    }
  }
}

//--------------------------------------------------------------------------
// Main
int main()
{
  n=0;
  nback=0;
  nfore=0;
  TestThread t1;
  TestThread t2;

  for(;;)
  {
    for(int i=0; i<1000; i++)
    {
      {
        MT::RLock lock(mutex);
	{
	  // Lock it again
	  MT::RLock lock2(mutex);
	  n++;
	  n--;
	  nfore++;
	}
      }
    }

    MT::RLock lock(mutex);
    cout << "N is " << n << " (" << nback << " in bg, " 
	 << nfore << " in fg)\n";
  }
  return 0;  
}




