//==========================================================================
// ObTools::Cache: test-timeout-age.cc
//
// Test harness for cache library - Time-since-last-used eviction
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-cache.h"
using namespace std;
using namespace ObTools;
using namespace ObTools::Cache;

//--------------------------------------------------------------------------
// Main

int main()
{
  AgeTimeoutCache<string, string> cache(5);

  cache.add("foo", "FOO");
  cache.dump(cout, true); 
  sleep(2);

  cache.add("bar", "BAR");
  cache.dump(cout, true);

  for(int i=0; i<10; i++)
  {
    cache.tidy();
    cache.touch("foo");
    cache.dump(cout, true);
    sleep(1);
  }

  return 0;  
}




