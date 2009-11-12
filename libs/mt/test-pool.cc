//==========================================================================
// ObTools::MT: test-pool.cc
//
// Test harness for thread pools
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-mt.h"
#include <cstdlib>
#include <iostream>

using namespace std;
using namespace ObTools::MT;

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
  cout << n << " finished" << endl;
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
      cout << "No spare threads" << endl;
      ObTools::MT::Thread::sleep(1);
    }
  }

  cout << "Shutting down" << endl;
  pool.shutdown();
  cout << "Done" << endl;

  return 0;  
}




