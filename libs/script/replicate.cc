//==========================================================================
// ObTools::Script: replicate.cc
//
// Replicated action
//   <replicate copies="5" spread="1"/>
//   copies: Number of copies
//   spread: Time between copy starts
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-script.h"

namespace ObTools { namespace Script {

//------------------------------------------------------------------------
// Constructor
ReplicatedAction::ReplicatedAction(Action::CP cp): Action(cp), started(0)
{
  copies = xml.get_attr_int("copies", 1);
  spread = Time::Duration(xml.get_attr("spread","0"));
}

//------------------------------------------------------------------------
// Tick action
// Returns whether still active
bool ReplicatedAction::tick()
{
  // If less started than wanted, start some
  while (started < copies)
  {
    // Check we're allowed to
    if (!started 
     || (script.now >= last_start && script.now-last_start >= spread))
    {
      // Create SequenceAction child using our own XML as the model 
      actions.push_back(new SequenceAction(Action::CP(script, xml)));
      started++;
      last_start = script.now;
    }

    // See if we need to wait
    if (!!spread) break;
  }

  // Tick all actions, deleting any that have finished
  for(list<Action *>::iterator p = actions.begin(); p!=actions.end();)
  {
    list<Action *>::iterator q = p++;  // Protect from deletion
    Action *a = *q;
    if (!a->tick())
    {
      actions.erase(q);
      delete a;
    }
  }

  // Check for end - everything started and all copies finished
  return started < copies || !actions.empty();
}

//------------------------------------------------------------------------
// Destructor
ReplicatedAction::~ReplicatedAction()
{
  for(list<Action *>::iterator p = actions.begin(); p!=actions.end();++p)
    delete *p;
}


}} // namespaces
