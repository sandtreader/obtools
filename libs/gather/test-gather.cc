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

  // Insert some more, one from the end
  uint32_t n2 = 0x01234567;
  buffer.insert((Gather::data_t *)&n2, 4, 2);

  // Chop 8 bytes off the end
  cout << "After limit: " << buffer.limit(buffer.get_length()-8) << endl;

  // Consume 5 bytes from the start
  buffer.consume(5);

  // Dump it
  buffer.dump(cout, true);

  // Output it to stdout via writev
  struct iovec io[4];
  writev(1, io, buffer.fill(io));

  return 0;  
}




