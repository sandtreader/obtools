//==========================================================================
// ObTools::MT: thread.cc
//
// Thread wrapper
//
// Copyright (c) 2003-2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-mt.h"
#include "ot-gen.h"

namespace ObTools { namespace MT {

//--------------------------------------------------------------------------
// Start - note separate from default constructor to allow you time to create
// parameters in subclass constructors
// Returns whether successful
bool Thread::start()
{
  running = true;  // before it runs - otherwise it could delete us again
                   // before we've even started!
                   // Set before trying to create the thread in order to avoid
                   // race condition with _thread_start
  mythread = make_unique<thread>([this](){
    this->run();
    this->running = false;
  });
  if (!mythread)
    running = false;
  return running;
}

//--------------------------------------------------------------------------
// Set priority - higher numbers have higher priority
// realtime sets SCHED_RR if set
// Whether successful (may fail if realtime requested when not root)
bool Thread::set_priority(int priority, bool realtime)
{
  if (!mythread)
    return false;

  auto param = sched_param{0};
  param.sched_priority = priority;
  return !pthread_setschedparam(mythread->native_handle(),
                                realtime ? SCHED_RR : SCHED_OTHER,
                                &param);
}

//--------------------------------------------------------------------------
// Cancel - ask it to stop
void Thread::cancel()
{
  if (mythread)
  {
    // Try to cancel if not already stopped
    running = false;

    // Join to make sure it has cleanly finished before we exit
    join();
  }
}

//--------------------------------------------------------------------------
// Destructor - ask it to cancel if started
Thread::~Thread()
{
  cancel();
}

}} // namespaces
