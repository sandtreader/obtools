//==========================================================================
// ObTools::MT: test-pool.cc
//
// Test harness for thread pools
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#include "ot-mt.h"
#include <cstdlib>
#include <iostream>
#include <unistd.h>

using namespace std;

//--------------------------------------------------------------------------
// Test thread class
class TestThread: public ObTools::MT::PoolThread
{
public:
  int n;

  virtual void run();
  TestThread(ObTools::MT::PoolReplacer<TestThread>& _rep):
    PoolThread(_rep) {}
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
  ObTools::MT::ThreadPool<TestThread> pool(1,10);

  // Keep pulling spare threads off the pool and kicking them
  for(int i=0; i<20; )
  {
    TestThread *t = pool.remove();
    if (t)
    {
      t->n = ++i;
      cout << "Kicking thread " << t->n << endl;
      t->kick();
    }
    else
    {
      cout << "No spare threads\n";
      sleep(1);
    }
  }
  return 0;  
}




