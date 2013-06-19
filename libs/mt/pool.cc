//==========================================================================
// ObTools::MT: pool.cc
//
// Thread pool implementation (mostly in templates in ot-mt.h, actually)
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-mt.h"

namespace ObTools { namespace MT {

//--------------------------------------------------------------------------
// C thread start function
// Modified version for pool threads which sleep and return - q.v. thread.cc
// Takes 'self' as argument and bounces on to (virtual) run()
static void *_pool_start(void *arg)
{
  PoolThread *self = *static_cast<PoolThread **>(arg);

  for(;;)
  {
    // Wait for inuse CV
    self->in_use.wait();

    // Check for die request
    if (self->dying) break;

    // Call virtual run() in subclass
    self->run();

    if (self->dying) break;

    // Make myself out of use again
    self->in_use.clear();

    // Return to pool
    self->replacer.replace(self);
  }

  // Dying
  self->running = false;
  pthread_exit(NULL);
  return NULL;
}

//--------------------------------------------------------------------------
// Constructor - automatically starts thread in !in_use state
PoolThread::PoolThread(IPoolReplacer& _replacer): 
  Thread(), replacer(_replacer), in_use(false), dying(false) 
{
  self = this;
  pthread_create(&thread, NULL, _pool_start, &self);
  valid = true;
  running = true;
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
  if (valid && running)
  {
    dying = true;
    in_use.signal();  // Release from wait
    if (wait)
    {
      if (!joined) pthread_join(thread, NULL);  // Wait for it to die
      joined = true;
    }
  }
}


}} // namespaces



