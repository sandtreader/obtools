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

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  int n = 0;
  int bits = 1;
  int bsize = 16;
  int prob = 0;

  if (argc > 1) n = atoi(argv[1]);
  if (argc > 2) bits = atoi(argv[2]);
  if (argc > 3) bsize = atoi(argv[3]);
  if (argc > 4) prob = atoi(argv[4]);

  // Create hash table with default types
  Hash::Table<> hash(bits, bsize);

  if (n)
  {
    cout << "Creating " << n << " entries\n";
    if (prob) cout << "Deleting with " << prob << "% probability\n";

    // Fill it up with stuff
    int i;
    for(i=0; i<n; i++)
    {
      uint32_t id;

      // Check it isn't already there
      do
      {
	id = (uint32_t)(random()^(random()<<1));
      } while (hash.lookup(id) != Hash::INVALID_INDEX);

      // Try to add it
      if (!hash.add(id, i))
      {
	cerr << "Adding failed after " << i << " entries\n";
	break;
      }

      // Read it back
      int32_t j = hash.lookup(id);
      if (i != j)
	cerr << "Lookup of " << id << " failed - expecting " 
	     << i << " got " << j << endl;

      // Delete according to probability
      if (prob && random()%100<prob) hash.remove(id);
    }
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




