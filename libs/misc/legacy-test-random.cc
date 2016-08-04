//==========================================================================
// ObTools::Text: test-random.cc
//
// Test harness for Misc library random functions
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-misc.h"
#include <iostream>
#include <map>

using namespace std;
using namespace ObTools;

#define NUM_TRIES 100000

//--------------------------------------------------------------------------
// Main

int main()
{
  Misc::Random r;

  cerr << "Generating " << NUM_TRIES << " random numbers\n";

  // Map of counts for each number
  map<uint32_t, uint32_t> counts;
  uint32_t collisions = 0;
  uint32_t worst = 0;

  for(int i=1; i<=NUM_TRIES; i++)
  {
    uint32_t n = r.generate_32();
    counts[n]++;
    if (counts[n] > 1)
    {
      if (counts[n] > worst) worst = counts[n];
      collisions++;
    }
  }

  if (collisions)
  {
    cerr << collisions << " collisions\n";
    cerr << "Worst repeat count is " << worst << endl;

    // Not very mathematical, but less than 1 in 1000 collisions is
    // probably OK
    return (collisions > NUM_TRIES/1000)?2:0;
  }
  else
  {
    cerr << "No collisions\n";
    return 0;
  }
}




