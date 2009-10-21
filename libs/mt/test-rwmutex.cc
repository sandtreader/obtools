//==========================================================================
// ObTools::MT: test-rwmutex.cc
//
// Test harness for readers/writer mutex functions
//
// Copyright (c) 2007 xMill Consulting Limited.  All rights reserved
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
MT::RWMutex mutex;
int n;  
int n_reads, n_writes;
int n_bad_reads;
int n_bad_writes;

//--------------------------------------------------------------------------
// Reader thread class
class ReaderThread: public MT::Thread
{
  virtual void run();

public:
  ReaderThread() { start(); }
};

void ReaderThread::run()
{
  for(;;)
  {
    MT::RWReadLock lock(mutex);

    // Lock again to test recursion on reads 
    {
      MT::RWReadLock lock2(mutex);
    }
    
    // Check if we see its inconsistent state
    if (n) n_bad_reads++;
    n_reads++;
  }
}

//--------------------------------------------------------------------------
// Writer thread class
class WriterThread: public MT::Thread
{
  virtual void run();

public:
  WriterThread() { start(); }
};

void WriterThread::run()
{
  for(;;)
  {
    {
      MT::RWWriteLock lock(mutex);

      // Lock it again to test recursion on writes
      {
	MT::RWWriteLock lock2(mutex);
      }

      // And again to test recursion on reads within writes
      {
	MT::RWReadLock lock3(mutex);
      }

      // Make sure it's not inconsistent already (another writer is here)
      if (n) n_bad_writes++;

      // Set to 'inconsistent' value
      n = 1;
      for(volatile int i=0; i<1000; i++)
	;
      n = 0;
    }

    n_writes++;
    for(volatile int i=0; i<1000; i++)
      ;
  }
}

//--------------------------------------------------------------------------
// Main
int main()
{
  n=0;
  n_reads=0;
  n_writes=0;
  n_bad_reads = 0;
  n_bad_writes = 0;

  ReaderThread r1;
  ReaderThread r2;

  WriterThread w1;
  WriterThread w2;

  for(;;)
  {
    cout << n_writes << " writes, " << n_bad_writes << " inconsistent; "
	 << n_reads << " reads, " << n_bad_reads << " inconsistent\n";
    MT::Thread::sleep(1);
  }

  return 0;  
}




