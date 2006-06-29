//==========================================================================
// ObTools::MT: test-thread.cc
//
// Test harness for thread functions
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-mt.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <stdlib.h>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Test thread class
class TestThread: public MT::Thread
{
  int n;
  virtual void run();

public:
  TestThread(int _n): n(_n) { start(); }
};

void TestThread::run()
{
  int i;
  for(i=0; i<10; i++)
  {
    cout << n << ": " << i << endl;
    sleep(1);
  }
  cout << n << " finished" << endl;
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  int numthreads = 10;
  if (argc > 1) numthreads = atoi(argv[1]);

  cout << numthreads << " threads:" << endl;

  vector<TestThread *> threads(numthreads);

  int i;
  cout << "Starting:" << endl;
  for(i=0; i<numthreads; i++)
    threads[i] = new TestThread(i);

  cout << "Started:" << endl;
  MT::Thread::sleep(5);

  cout << "Cancelling half:" << endl;
  for(i=0; i<numthreads/2; i++)
    threads[i]->cancel();

  cout << "Joining:" << endl;
  for(i=0; i<numthreads; i++)
    threads[i]->join();

  cout << "Done" << endl;
  return 0;  
}




