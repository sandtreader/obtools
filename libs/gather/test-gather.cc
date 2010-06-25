//==========================================================================
// ObTools::Gather: test-gather.cc
//
// Test harness for gather buffer library
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-gather.h"
#include "iostream"
#include <stdlib.h>
#include <string.h>

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  Gather::Buffer buffer(1);

  const char *stuff = "Hello, world!";

  // Add some referenced data
  buffer.add((Gather::data_t *)stuff, strlen(stuff));

  // Add some internal data
  Gather::Segment& seg = buffer.add(16);
  for(unsigned int i=0; i<seg.length; i++) seg.data[i]=i;

  // Insert some reference data
  uint32_t n = 0xDEADBEEF;
  buffer.insert((Gather::data_t *)&n, 4);

  // Dump it
  buffer.dump(cout, true);

  return 0;  
}




