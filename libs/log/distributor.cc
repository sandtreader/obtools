//==========================================================================
// ObTools::Log: distributor.cc
//
// Log distributor
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-log.h"
#include "ot-text.h"

namespace ObTools { namespace Log {

//--------------------------------------------------------------------------
// Connect a channel under the given name
// Takes ownership and will dispose of it unless disconnected again
void Distributor::connect(const string& name, Channel *chan)
{
  MT::Lock lock(mutex);
  channels[name] = chan;
}

//--------------------------------------------------------------------------
// Disconnect the named channel but don't dispose of it
// Returns the channel if found, 0 if not
Channel *Distributor::disconnect(const string& name)
{
  MT::Lock lock(mutex);

  map<string, Channel *>::iterator p = channels.find(name);
  if (p != channels.end())
  {
    Channel *c = p->second;
    channels.erase(p);
    return c;
  }
  else return 0;
}

//--------------------------------------------------------------------------
// Disconnect and dispose of the named channel
// Whether it was found
bool Distributor::dispose(const string& name)
{
  MT::Lock lock(mutex);
  Channel *c = disconnect(name);
  if (c)
  {
    delete c;
    return true;
  }
  else return false;
}

//--------------------------------------------------------------------------
// Log a message
void Distributor::log(Message& msg)
{
  MT::Lock lock(mutex);

  // Send to all channels with max level higher or equal to this, with
  // matching pattern (or none)
  map<string, Channel *>::iterator p;
  for(p=channels.begin(); p!=channels.end(); p++)
  {
    Channel *c = p->second;
    if (c->level >= msg.level
     && (!c->pattern.size() || Text::pattern_match(c->pattern, msg.text))) 
      c->log(msg);
  }
}

//--------------------------------------------------------------------------
// Destructor - disposes of all registered channels
Distributor::~Distributor()
{
  MT::Lock lock(mutex);

  map<string, Channel *>::iterator p;
  for(p=channels.begin(); p!=channels.end(); p++)
  {
    Channel *c = p->second;
    delete c;
  }
}


}} // namespaces



