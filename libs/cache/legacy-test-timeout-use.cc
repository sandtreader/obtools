//==========================================================================
// ObTools::Cache: test-timeout-use.cc
//
// Test harness for cache library - Time-since-last-used eviction
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-cache.h"
using namespace std;
using namespace ObTools;
using namespace ObTools::Cache;

//--------------------------------------------------------------------------
// Main

int main()
{
  UseTimeoutCache<string, string> cache(5);

  cache.add("foo", "FOO");
  cache.dump(cout, true);
  this_thread::sleep_for(chrono::seconds{2});

  cache.add("bar", "BAR");
  cache.dump(cout, true);

  for(int i=0; i<10; i++)
  {
    cache.tidy();
    cache.touch("foo");
    cache.dump(cout, true);
    this_thread::sleep_for(chrono::seconds{1});
  }

  return 0;
}
