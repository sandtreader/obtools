//==========================================================================
// ObTools::MT: pool.cc
//
// Thread pool implementation (mostly in templates in ot-mt.h, actually)
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-mt.h"

namespace ObTools { namespace MT {

//--------------------------------------------------------------------------
// C thread start function
// Modified version for pool threads which sleep and return - q.v. thread.cc
// Takes 'self' as argument and bounces on to (virtual) run()
static void *_pool_start(void *arg)
{
  PoolThread *self = *(PoolThread **)arg;

  for(;;)
  {
    // Wait for inuse CV
    self->in_use.wait();

    // Call virtual run() in subclass
    self->run();

    // Make myself out of use again
    self->in_use.clear();

    // Return to pool
    self->replacer.replace(self);
  }
}

//--------------------------------------------------------------------------
// Constructor - automatically starts thread in !in_use state
PoolThread::PoolThread(IPoolReplacer& _replacer): replacer(_replacer)
{
  self = this;
  pthread_create(&thread, NULL, _pool_start, &self) == 0; 
}

//--------------------------------------------------------------------------
// Kick thread into life  
void PoolThread::kick()
{
  in_use.signal();  // Tell myself to start
}


}} // namespaces



