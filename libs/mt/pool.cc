//==========================================================================
// ObTools::MT: pool.cc
//
// Thread pool implementation (mostly in templates in ot-mt.h, actually)
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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
PoolThread::PoolThread(IPoolReplacer& _replacer): Thread(), replacer(_replacer)
{
  self = this;
  pthread_create(&thread, NULL, _pool_start, &self); 
  running = true;
}

//--------------------------------------------------------------------------
// Kick thread into life  
void PoolThread::kick()
{
  in_use.signal();  // Tell myself to start
}


}} // namespaces



