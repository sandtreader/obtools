//==========================================================================
// ObTools::Ring: test-ring.cc
//
// Test harness for ring buffer library
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-ring.h"
#include "ot-mt.h"
#include "iostream"
#include <stdlib.h>

using namespace std;
using namespace ObTools;

// Handy typedef for our test ring buffer - just stores integers
typedef Ring::Buffer<int> TestBuffer;

//--------------------------------------------------------------------------
// Read thread class
class ReadThread: public MT::Thread
{
  TestBuffer& buffer;
  virtual void run();

public:
  ReadThread(TestBuffer& _buffer): buffer(_buffer)
  { start(); }
};

void ReadThread::run()
{
  int next_n = 0;

  // Read as fast as possible
  for(;;)
  {
    int n;
    if (buffer.get(n))
    {
      if (n != next_n)
	cerr << "Out of phase - expected " << next_n << " got " << n << endl;

      //      cout << n << endl;
      next_n = n+1;
    }
    //else cout << "-";
  }
}

//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  int length = 100;
  if (argc > 1) length = atoi(argv[1]);
  TestBuffer buffer(length);
  cerr << "Buffer length " << length << endl;

  // Start reader thread
  ReadThread read_thread(buffer);

  // Write as fast as possible
  int n=0;
  for(;;)
  {
    if (buffer.put(n)) 
      n++;
    //else cout << "+";
  }

  return 0;  
}




