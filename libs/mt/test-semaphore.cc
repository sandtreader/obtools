//==========================================================================
// ObTools::MT: test-semaphore.cc
//
// Test harness for semaphores
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include <gtest/gtest.h>
#include "ot-mt.h"

using namespace std;
using namespace ObTools;

//--------------------------------------------------------------------------
// Tests
TEST(SemaphoreTest, TestThreadBlocksOnWaitBeforeSignal)
{
  MT::Semaphore sem;
  int hit = 0;

  thread t([&sem, &hit]
           {
             sem.wait();
             hit++;
           });

  this_thread::sleep_for(chrono::milliseconds{100});
  ASSERT_EQ(0, hit) << "Hit before signal";
  sem.signal();
  t.join();
  ASSERT_EQ(1, hit) << "Hit not set after signal";
}

TEST(SemaphoreTest, TestPreseededSemaphoreDoesntNeedSignal)
{
  MT::Semaphore sem(1);
  int hit = 0;

  thread t([&sem, &hit]
           {
             sem.wait();
             hit++;
           });

  t.join();
  ASSERT_EQ(1, hit) << "Hit not set with preseed";
}

TEST(SemaphoreTest, TestSemaphoreThreadSafety)
{
  MT::Semaphore sem_up, sem_down;
  int hit = 0;

  thread up([&]
            {
              for(auto i=0; i<10000; i++)
              {
                sem_up.wait();
                hit++;
                ASSERT_EQ(1, hit) << "up found bad hit";
                sem_down.signal();
              }
            });

  thread down([&]
            {
              for(auto i=0; i<10000; i++)
              {
                sem_down.wait();
                hit--;
                ASSERT_EQ(0, hit) << "down found bad hit";
                sem_up.signal();
              }
            });

  sem_up.signal();
  up.join();
  down.join();
  ASSERT_EQ(0, hit) << "Hit not 0 at end";
}

TEST(SemaphoreTest, TestSemaphoreHolder)
{
  MT::Semaphore sem;
  int hit = 0;

  thread t1([&sem, &hit]
           {
             MT::SemaphoreHolder holder(sem);
             hit++;
           });

  thread t2([&sem, &hit]
           {
             MT::SemaphoreHolder holder(sem);
             hit++;
           });

  this_thread::sleep_for(chrono::milliseconds{100});
  ASSERT_EQ(0, hit) << "Threads aren't waiting";
  sem.signal();

  t1.join();
  t2.join();
  ASSERT_EQ(2, hit);
}

TEST(SemaphoreTest, TestMultipleSignalWakesAll)
{
  MT::Semaphore sem;
  int hit = 0;

  thread t1([&sem, &hit]
           {
             sem.wait();  // Note - doesn't give it back
             hit++;
           });

  thread t2([&sem, &hit]
           {
             sem.wait();
             hit++;
           });

  this_thread::sleep_for(chrono::milliseconds{100});
  ASSERT_EQ(0, hit) << "Threads aren't waiting";
  sem.signal(2);

  t1.join();
  t2.join();
  ASSERT_EQ(2, hit);
}

TEST(SemaphoreTest, TestSingleSignalUsingMultipleCallWakesOne)
{
  MT::Semaphore sem;
  int hit = 0;

  thread t1([&sem, &hit]
           {
             sem.wait();  // Note - doesn't give it back
             hit++;
           });

  thread t2([&sem, &hit]
           {
             sem.wait();
             hit++;
           });

  this_thread::sleep_for(chrono::milliseconds{100});
  ASSERT_EQ(0, hit) << "Threads aren't waiting";

  // Wake one
  sem.signal(1);
  this_thread::sleep_for(chrono::milliseconds{100});
  ASSERT_EQ(1, hit);

  // Then the other
  sem.signal(1);
  this_thread::sleep_for(chrono::milliseconds{100});
  ASSERT_EQ(2, hit);

  t1.join();
  t2.join();
}


//--------------------------------------------------------------------------
// Main
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
