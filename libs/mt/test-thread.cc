//==========================================================================
// ObTools::MT: test-thread.cc
//
// Test harness for thread functions
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
  cout << n << " finished\n";
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  int numthreads = 10;
  if (argc > 1) numthreads = atoi(argv[1]);

  cout << numthreads << " threads:\n";

  vector<TestThread *> threads(numthreads);

  int i;
  cout << "Starting:\n";
  for(i=0; i<numthreads; i++)
    threads[i] = new TestThread(i);

  cout << "Started:\n";
  sleep(5);

  cout << "Cancelling half:\n";
  for(i=0; i<numthreads/2; i++)
    threads[i]->cancel();

  cout << "Joining:\n";
  for(i=0; i<numthreads; i++)
    threads[i]->join();

  cout << "Done\n";
  return 0;  
}




