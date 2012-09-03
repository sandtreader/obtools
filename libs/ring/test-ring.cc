#//==========================================================================
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

#define DEFAULT_BUFFER_LENGTH 100
#define DEFAULT_ITERATIONS 10000000

using namespace std;
using namespace ObTools;

// Handy typedef for our test ring buffer - just stores integers
typedef Ring::Buffer<int> TestBuffer;

//--------------------------------------------------------------------------
// Read thread class
class ReadThread: public MT::Thread
{
  TestBuffer& buffer;
  bool running;
  virtual void run();

public:
  int errors;
  ReadThread(TestBuffer& _buffer): buffer(_buffer), running(true), errors(0)
  { start(); }

  void stop() { running = false; }
};

void ReadThread::run()
{
  int next_n = 0;

  // Read as fast as possible
  while (running)
  {
    int n;
    if (buffer.get(n))
    {
      if (n != next_n)
      {
	cerr << "Out of phase - expected " << next_n << " got " << n << endl;
        errors++;
      }

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
  int iterations = DEFAULT_ITERATIONS;
  if (argc > 1) iterations = atoi(argv[1]);

  int length = DEFAULT_BUFFER_LENGTH;
  if (argc > 2) length = atoi(argv[2]);

  TestBuffer buffer(length);
  cerr << "Buffer length " << length << endl;
  cerr << iterations << " iterations\n";

  // Start reader thread
  ReadThread read_thread(buffer);

  // Write as fast as possible
  int n=0;
  while (n < iterations)
  {
    if (buffer.put(n))
      n++;
    //else cout << "+";
  }

  read_thread.stop();
  cerr << read_thread.errors << " errors\n";
  return read_thread.errors?2:0;
}




