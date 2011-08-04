//==========================================================================
// ObTools::MT: test-qqueue.cc
//
// Test harness for data queue
//
// Copyright (c) 2010 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-mt.h"
#include <cstdlib>
#include <iostream>

using namespace std;

ObTools::MT::DataQueue dq;

//--------------------------------------------------------------------------
// Receiver thread class
class ReceiveThread: public ObTools::MT::Thread
{
  virtual void run();

public:
  ReceiveThread() { start(); }
};

void ReceiveThread::run()
{
  // Just pull numbers off the queue
  for(;;)
  {
    unsigned char buf[16];
    int n = dq.read(buf, 16);
    cout << "RX: " << string((char *)buf, n) << endl;
    if (n<16) break;
  }
  
  cout << "EOF\n";
}

//--------------------------------------------------------------------------
// Main

int main()
{
  // Start receiver
  ReceiveThread receiver;

  // Transmit for a while
  for(int i=0; i<1000; i++)
  {
    dq.write((const unsigned char *)"Hello, world!", 13);
    ObTools::MT::Thread::usleep(10000);
  }

  // Mark EOF and let it react
  dq.close();
  ObTools::MT::Thread::sleep(1);

  // Send one more to make sure destructor cleans up
  dq.write((const unsigned char *)"Goodbye, world!", 15);

  return 0;  
}




