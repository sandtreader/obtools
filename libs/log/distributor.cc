//==========================================================================
// ObTools::Log: distributor.cc
//
// Log distributor
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-log.h"

namespace ObTools { namespace Log {

//--------------------------------------------------------------------------
// Connect a channel under the given name
// Takes ownership and will dispose of it unless disconnected again
void Distributor::connect(const string& name, Channel *chan)
{
  MT::Lock lock(mutex);
}

//--------------------------------------------------------------------------
// Disconnect the named channel but don't dispose of it
// Returns the channel if found, 0 if not
Channel *Distributor::disconnect(const string& name)
{
  MT::Lock lock(mutex);
}

//--------------------------------------------------------------------------
// Disconnect and dispose of the named channel
// Whether it was found
bool Distributor::dispose(const string& name)
{
  MT::Lock lock(mutex);
}

//--------------------------------------------------------------------------
// Log a message
void Distributor::log(Message& msg)
{
  MT::Lock lock(mutex);
}

//--------------------------------------------------------------------------
// Destructor - disposes of all registered channels
Distributor::~Distributor()
{
  MT::Lock lock(mutex);
}


}} // namespaces



