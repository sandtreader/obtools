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
    MT::RLock lock(mutex);
    {
      // Lock it again
      MT::RLock lock2(mutex);
      n++;
      n--;
    }
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
      MT::RLock lock(mutex);
      {
	// Lock it again
	MT::RLock lock2(mutex);
	n++;
	n--;
      }
    }

    cout << "N is " << n << endl;
  }
  return 0;  
}




