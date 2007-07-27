//==========================================================================
// ObTools::Hash: test-hash.cc
//
// Test harness for hash library
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-hash.h"
#include "iostream"

using namespace std;
using namespace ObTools;

// Globals
int nruns = 0;
int prob = 0;
MT::Mutex mutex;

//--------------------------------------------------------------------------
// Test thread class
class TestThread: public MT::Thread
{
  int n;
  Hash::Table<>& hash;
  virtual void run();

public:
  bool running;
  TestThread(int _n, Hash::Table<>& _hash): n(_n), hash(_hash), running(true) 
  { start(); }
};

void TestThread::run()
{
  cout << "Thread " << n << " creating " << nruns << " entries\n";
  if (prob) cout << "Deleting with " << prob << "% probability\n";

  // Fill it up with stuff
  int i;
  for(i=0; i<nruns; i++)
  {
    uint32_t id;

    // Check it isn't already there
    do
    {
      id = (uint32_t)(rand()^(rand()<<1));
    } while (hash.lookup(id) != Hash::INVALID_INDEX);

    // Try to add it
    {
      MT::Lock lock(mutex);
      if (!hash.add(id, i))
      {
	cerr << "Adding failed after " << i << " entries\n";
	break;
      }
    }

    // Read it back
    int32_t j = hash.lookup(id);
    if (i != j)
      cerr << "Lookup of " << id << " failed - expecting " 
	   << i << " got " << j << endl;

    // Delete according to probability
    if (prob && rand()%100<prob) 
    {
      MT::Lock lock(mutex);
      hash.remove(id);
    }
  }

  cout << "Thread " << n << " finished\n";
  running = false;
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  int nthreads = 0;
  int bits = 1;
  int bsize = 16;

  if (argc > 1) nthreads = atoi(argv[1]);
  if (argc > 2) nruns = atoi(argv[2]);
  if (argc > 3) bits = atoi(argv[3]);
  if (argc > 4) bsize = atoi(argv[4]);
  if (argc > 5) prob = atoi(argv[5]);

  // Create hash table with default types
  cout << "Creating table with " << bits << " top bits, " 
       << (1<<bits) << " blocks of " << bsize << " entries\n";
  Hash::Table<> hash(bits, bsize);
  cout << "Total capacity: " << hash.capacity() << " entries\n";
  cout << "Total memory: " << (hash.memory() >> 20) << "MB\n";

  if (nthreads)
  {
    cout << "Starting " << nthreads << " threads:" << endl;

    vector<TestThread *> threads(nthreads);

    int i;
    cout << "Starting:" << endl;
    for(i=0; i<nthreads; i++)
      threads[i] = new TestThread(i, hash);

    cout << "Waiting:" << endl;
    for(;;)
    {
      // Check the hash is OK
      if (!hash.check(cerr)) 
      {
	cerr << "Hash table is invalid!\n";
	return 2;
      }

      bool any_running = false;
      for(i=0; i<nthreads; i++)
	if (threads[i]->running)
	  any_running = true;

      if (!any_running) break;
    }

    cout << "Joining threads:" << endl;
    for(i=0; i<nthreads; i++)
      threads[i]->join();
  }
  else
  {
    // Specific test cases
    hash.add(0, 1000);
    hash.add(bsize, 1001);
    hash.add(bsize*2, 1002);

    hash.add(bsize-1, 2000);
    hash.add(bsize*2-1, 2001);
    hash.add(bsize*3-1, 2002);

    hash.add(bsize/2, 3000);

    hash.remove(0);            // First in chain
    hash.remove(bsize*2-1);    // Middle of chain
    hash.remove(bsize/2);      // By itself
  }

  // Validate
  cout << "Validating hash table\n";
  if (!hash.check(cerr)) 
  {
    cerr << "Hash table is invalid!\n";
    return 2;
  }
  cout << "Hash table OK\n";

  // Get stats
  cout << "Hash table statistics:\n";
  Hash::Stats stats;
  hash.get_stats(stats);

  cout << "    Total entries: " << stats.entries << endl;
  cout << "   Total capacity: " << hash.capacity() << endl;
  cout << " Overall fullness: " << 100*stats.entries/hash.capacity()
	      << "%\n";
  cout << "     Max fullness: " << stats.max_fullness << "%\n";
  cout << "    Longest chain: " << stats.max_chain << endl;

  // Dump out
  //hash.dump(cout);

  return 0;  
}




