//==========================================================================
// ObTools::MT: thread.cc
//
// Thread wrapper
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-mt.h"

namespace ObTools { namespace MT {

//--------------------------------------------------------------------------
// C thread start function
// Takes 'self' as argument and bounces on to (virtual) run()
static void *_start(void *arg)
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
// Start - note separate from default constructor to allow you time to create
// parameters in subclass constructors
// Returns whether successful
bool Thread::start()
{
  self = this;
  return pthread_create(&thread, NULL, _start, &self) == 0; 
}

//--------------------------------------------------------------------------
// Join - caller waits for this thread to end
void Thread::join()
{
  if (thread) pthread_join(thread, NULL);
}

//--------------------------------------------------------------------------
// Detach - let it die silently when it ends, we aren't going to join with it
void Thread::detach()
{
  if (thread) pthread_detach(thread);
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



