//==========================================================================
// ObTools::MT: test-pool.cc
//
// Test harness for thread pools
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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

int main()
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
      ObTools::MT::Thread::sleep(1);
    }
  }

  cout << "Shutting down\n";
  pool.shutdown();
  cout << "Done\n";

  return 0;  
}




