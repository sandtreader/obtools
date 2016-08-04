//==========================================================================
// ObTools::Script: thread.cc
//
// Thread action - runs its contents as a separate real processor thread
// Sleep time gives time (us) between child ticks
// <thread sleep="10000">
//   ...
// </thread>
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-script.h"
#include "ot-log.h"
#define DEFAULT_SLEEP_TIME 10000

namespace ObTools { namespace Script {

//--------------------------------------------------------------------------
// Constructor
ThreadAction::ThreadAction(const CP& cp): SequenceAction(cp), thread(0)
{
  sleep_time = xml.get_attr_int("sleep", DEFAULT_SLEEP_TIME);
}

//--------------------------------------------------------------------------
// Tick action
// Returns whether still active
bool ThreadAction::tick(Context& con)
{
  // Start thread if not already started
  if (!thread) thread = new ActionThread(*this, con);

  // Check if the thread is still running
  return !!*thread;
}

//--------------------------------------------------------------------------
// Run thread
void ThreadAction::run_thread(Context& con)
{
  while (SequenceAction::tick(con))
    this_thread::sleep_for(chrono::microseconds{sleep_time});
}

//--------------------------------------------------------------------------
// Destructor
ThreadAction::~ThreadAction()
{
  if (thread) delete thread;
}

}} // namespaces
