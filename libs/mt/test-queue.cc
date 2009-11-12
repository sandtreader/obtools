//==========================================================================
// ObTools::MT: test-queue.cc
//
// Test harness for message queue functions
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-mt.h"
#include <cstdlib>
#include <iostream>

using namespace std;

ObTools::MT::Queue<int> mq;

//--------------------------------------------------------------------------
// Receiver thread class
class ReceiveThread: public ObTools::MT::Thread
{
  virtual void run();
  int n;

public:
  ReceiveThread(int _n): n(_n) { start(); }
};

void ReceiveThread::run()
{
  // Just pull numbers off the queue
  for(;;)
  {
    int m = mq.wait();
    cout << "RX(" << n << "): " << m << endl;

    // Sleep for sufficient time so that if we're working alone we
    // couldn't do all 10 in the second it takes between bursts -
    // but if other receivers are also running, we could
    // This demonstrates that plain MT::Queue does not distributed bursty
    // messages properly, hence why MT::MQueue exists
    ObTools::MT::Thread::usleep(120000);
  }
}

//--------------------------------------------------------------------------
// Transmitter thread class
class TransmitThread: public ObTools::MT::Thread
{
  virtual void run();
  int n;

public:
  TransmitThread(int _n): n(_n) { start(); }
};

void TransmitThread::run()
{
  // Post numbers onto the queue
  for(int i=0; i<10; i++)
  {
    for(int j=0; j<10; j++)
    {
      cout << "TX(" << n << "): " << 100*n+10*i+j << endl;
      mq.send(100*n+10*i+j);
    }
    sleep(1);
  }
}

//--------------------------------------------------------------------------
// Main

int main()
{
  // Start receivers
  ReceiveThread r1(1);
  ReceiveThread r2(2);
  ReceiveThread r3(3);

  // Then transmitters
  TransmitThread t1(1);
  TransmitThread t2(2);

  // Wait for transmits to finish, then kill receives
  t1.join();
  t2.join();

  r1.cancel();
  r2.cancel();
  r3.cancel();

  return 0;  
}




