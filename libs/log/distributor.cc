//==========================================================================
// ObTools::Log: distributor.cc
//
// Log distributor
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#include "ot-log.h"
#include "ot-text.h"

namespace ObTools { namespace Log {

//--------------------------------------------------------------------------
// Connect a channel
void Distributor::connect(Channel& chan)
{
  MT::Lock lock(mutex);
  channels.push_back(&chan);
}

//--------------------------------------------------------------------------
// Disconnect the given channel
void Distributor::disconnect(Channel& chan)
{
  MT::Lock lock(mutex);

  channels.remove(&chan);
}

//--------------------------------------------------------------------------
// Log a message
void Distributor::log(Message& msg)
{
  MT::Lock lock(mutex);

  // Send to all channels 
  list<Channel *>::iterator p;
  for(p=channels.begin(); p!=channels.end(); p++)
    (*p)->log(msg);
}

}} // namespaces



