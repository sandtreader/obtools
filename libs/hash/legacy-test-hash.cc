//==========================================================================
// ObTools::Hash: test-hash.cc
//
// Test harness for hash library
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-hash.h"
#include "iostream"

using namespace std;
using namespace ObTools;

// Handy typedef for type of hash to test
// Note:  We extend the size of the hash index, because we don't assume
// we have enough memory to provide full 32-bit coverage - otherwise we
// will get collisions between different IDs which will confuse the test
typedef Hash::Table<uint32_t, uint32_t, int16_t, int32_t> TestHash;

// Globals
int nthreads = 0;
int nruns = 0;
int nids = 1;
int prob = 0;

//--------------------------------------------------------------------------
// Test thread class
class TestThread: public MT::Thread
{
  int n;
  TestHash& hash;
  virtual void run();

public:
  bool running;
  TestThread(int _n, TestHash& _hash): n(_n), hash(_hash), running(true)
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
    vector<uint32_t> ids(nids);

    // Create nids ids
    for(int j=0; j<nids; j++)
    {
      // Check they aren't already there
      do
      {
        // Generate a random ID with thread uniqueness in lowest bits
        ids[j] = (static_cast<uint32_t>(rand()^(rand()<<1))/nthreads)
                 *nthreads+n;
      } while (hash.lookup(ids[j]) != Hash::INVALID_INDEX);

      // Now OK to add
      if (!hash.add(ids[j], i*10+j))
      {
        cout << "Adding failed after " << i << " entries\n";
        break;
      }
    }

    // Read them back
    for(int j=0; j<nids; j++)
    {
      int32_t i2 = hash.lookup(ids[j]);
      if (i2 != i*10+j)
      {
        cout << "Lookup of " << ids[j] << " failed - expecting "
             << i+j << " got " << i2 << endl;
        //        hash.dump(cout);
      }
    }

    // Delete according to probability
    for(int j=0; j<nids; j++)
    {
      if (prob && rand()%100<prob)
        hash.remove(ids[j]);
    }
  }

  cout << "Thread " << n << " finished\n";
  running = false;
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  int bits = 1;
  int bsize = 16;

  if (argc > 1) nthreads = atoi(argv[1]);
  if (argc > 2) nruns    = atoi(argv[2]);
  if (argc > 3) nids     = atoi(argv[3]);
  if (argc > 4) bits     = atoi(argv[4]);
  if (argc > 5) bsize    = atoi(argv[5]);
  if (argc > 6) prob     = atoi(argv[6]);

  // Create hash table with default types
  cout << "Creating table with " << bits << " top bits, "
       << (1<<bits) << " blocks of " << bsize << " entries\n";
  TestHash hash(bits, bsize);
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
      if (!hash.check(cout))
      {
        cout << "Hash table is invalid!\n";
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
  if (!hash.check(cout))
  {
    cout << "Hash table is invalid!\n";
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
  // hash.dump(cout);

  return 0;
}




