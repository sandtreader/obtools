//==========================================================================
// ObTools::Log: distributor.cc
//
// Log distributor
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-log.h"
#include "ot-text.h"

namespace ObTools { namespace Log {

//--------------------------------------------------------------------------
// Connect a channel
void Distributor::connect(Channel& chan)
{
#if !defined(_SINGLE)
  MT::Lock lock(mutex);
#endif

  channels.push_back(&chan);
}

//--------------------------------------------------------------------------
// Disconnect the given channel
void Distributor::disconnect(Channel& chan)
{
#if !defined(_SINGLE)
  MT::Lock lock(mutex);
#endif

  channels.remove(&chan);
}

//--------------------------------------------------------------------------
// Log a message
void Distributor::log(Message& msg)
{
#if !defined(_SINGLE)
  MT::Lock lock(mutex);
#endif

  // Send to all channels
  list<Channel *>::iterator p;
  for(p=channels.begin(); p!=channels.end(); p++)
    (*p)->log(msg);
}

}} // namespaces



