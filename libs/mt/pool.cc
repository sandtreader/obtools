//==========================================================================
// ObTools::MT: pool.cc
//
// Thread pool implementation (mostly in templates in ot-mt.h, actually)
//
// Copyright (c) 2003-2016 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-mt.h"
#include "ot-gen.h"

namespace ObTools { namespace MT {

//--------------------------------------------------------------------------
// Start
// Modified version for pool threads which sleep and return - q.v. thread.cc
// Takes 'self' as argument and bounces on to (virtual) run()
bool PoolThread::start()
{
  running.signal(true);
  mythread = make_unique<thread>([this](){
    while (true)
    {
      // Wait for inuse CV
      this->in_use.wait();

      // Check for die request
      if (this->dying)
        break;

      // Call virtual run() in subclass
      this->run();

      if (this->dying)
        break;

      // Make myself out of use again
      this->in_use.clear();

      // Return to pool
      this->replacer.replace(this);
    }
    running.signal(false);
  });
  if (!mythread)
    running.signal(false);
  return running;
}

//--------------------------------------------------------------------------
// Constructor - automatically starts thread in !in_use state
PoolThread::PoolThread(IPoolReplacer& _replacer):
  replacer{_replacer}, in_use{false}, dying{false}
{
  start();
}

//--------------------------------------------------------------------------
// Kick thread into life
void PoolThread::kick()
{
  in_use.signal();  // Tell myself to start
}

//--------------------------------------------------------------------------
// Request it to die.  If 'wait' is set, waits for thread to exit
void PoolThread::die(bool wait)
{
  if (is_running())
  {
    dying = true;
    in_use.signal();  // Release from wait
    if (wait)
    {
      if (mythread->joinable())
        mythread->join();
    }
  }
}

}} // namespaces
