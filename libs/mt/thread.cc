//==========================================================================
// ObTools::MT: thread.cc
//
// Thread wrapper
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#include "ot-mt.h"

namespace ObTools { namespace MT {

//--------------------------------------------------------------------------
// C thread start function
// Takes 'self' as argument and bounces on to (virtual) run()
extern "C" void *_thread_start(void *arg)
{
  Thread *self = *(Thread **)arg;

  // Call virtual run() in subclass
  self->run();

  // Clear thread for any future method calls
  self->thread = 0;

  // Die
  pthread_exit(NULL);
  return NULL;
}

//--------------------------------------------------------------------------
// Cancellation handler to unlock a mutex
extern "C" void _unlock_mutex(void *m)
{
  pthread_mutex_unlock((pthread_mutex_t *)m);
}

//--------------------------------------------------------------------------
// Start - note separate from default constructor to allow you time to create
// parameters in subclass constructors
// Returns whether successful
bool Thread::start()
{
  self = this;
  return pthread_create(&thread, NULL, _thread_start, &self) == 0; 
}

//--------------------------------------------------------------------------
// Cancel - ask it to stop
void Thread::cancel()
{
  if (thread)
  {
    pthread_cancel(thread);
    // Join to make sure it has cleanly finished before we exit
    if (thread) pthread_join(thread, NULL);
    thread = 0;
  }
}

//--------------------------------------------------------------------------
// Destructor - ask it to cancel if started
Thread::~Thread()
{
  cancel();
}

}} // namespaces



