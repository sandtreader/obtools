//==========================================================================
// ObTools::MT: thread.cc
//
// Thread wrapper
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-mt.h"
#if !defined(BORLAND)
#include <unistd.h>
#endif

namespace ObTools { namespace MT {

//--------------------------------------------------------------------------
// C thread start function
// Takes 'self' as argument and bounces on to (virtual) run()
extern "C" void *_thread_start(void *arg)
{
  Thread *self = *(Thread **)arg;

  // Call virtual run() in subclass
  self->run();

  // Stop running
  self->running = false;

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
  if (pthread_create(&thread, NULL, _thread_start, &self)) return false;
  valid = true;
  running = true;  // Set before it actually starts, in case caller checks
                   // before it runs - otherwise it could delete us again
                   // before we've even started!
  return true;
}

//--------------------------------------------------------------------------
// Set priority - higher numbers have higher priority
// realtime sets SCHED_RR if set
// Whether successful (may fail if realtime requested when not root)
bool Thread::set_priority(int priority, bool realtime)
{
  struct sched_param param;
  param.sched_priority = priority;
  return valid && !pthread_setschedparam(thread, 
					 realtime?SCHED_RR:SCHED_OTHER, 
					 &param);
}

//--------------------------------------------------------------------------
// Cancel - ask it to stop
void Thread::cancel()
{
  if (valid)
  {
    // Try to cancel if not already stopped
    if (running) pthread_cancel(thread);
    running = false;

    // Join to make sure it has cleanly finished before we exit
    if (!joined) pthread_join(thread, NULL);
    joined = true;
  }
}

//--------------------------------------------------------------------------
// Destructor - ask it to cancel if started
Thread::~Thread()
{
  cancel();
}

//--------------------------------------------------------------------------
// Sleep for given number of seconds
void Thread::sleep(int secs)
{
#if (defined(MINGW) || defined(BORLAND))
  // Use pthread_delay_np to provide cancellation point
  struct timespec interval;
  interval.tv_sec = secs;
  interval.tv_nsec = 0;
  pthread_delay_np(&interval);
#else
    ::sleep(secs);
#endif
}

//--------------------------------------------------------------------------
// Sleep for given number of micro-seconds
void Thread::usleep(int usecs)
{
#if (defined(MINGW) || defined(BORLAND))
  // Use pthread_delay_np to provide cancellation point
  struct timespec interval;
  interval.tv_sec = usecs/1000000;
  interval.tv_nsec = (usecs % 1000000)*1000;
  pthread_delay_np(&interval);
#else
  ::usleep(usecs);
#endif
}

}} // namespaces



