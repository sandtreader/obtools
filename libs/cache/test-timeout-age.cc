//==========================================================================
// ObTools::Cache: test-timeout-use.cc
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
  TimeoutAgeCache<string, string> cache(5);

  cache.add("foo", "FOO");
  cache.dump(cout); 
  sleep(2);

  cache.add("bar", "BAR");
  cache.dump(cout);

  for(int i=0; i<10; i++)
  {
    cache.tidy();
    cache.touch("foo");
    cache.dump(cout);
    sleep(1);
  }

  return 0;  
}




