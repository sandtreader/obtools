//==========================================================================
// ObTools::MT: test-queue.cc
//
// Test harness for message queue functions
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-mt.h"
#include <cstdlib>
#include <iostream>
#include <unistd.h>

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
    cout << "TX(" << n << "): " << 100*n+i << endl;
    mq.send(100*n+i);
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




