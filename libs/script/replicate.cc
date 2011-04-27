//==========================================================================
// ObTools::Script: replicate.cc
//
// Replicated action
//   <replicate copies="5" spread="1"/>
//   copies: Number of copies
//   spread: Time between copy starts
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
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
bool ReplicatedAction::tick(Context& con)
{
  // If less started than wanted, start some
  while (started < copies)
  {
    // Check we're allowed to
    if (!started 
     || (script.now >= last_start && script.now-last_start >= spread))
    {
      // Create SequenceAction child using our own XML as the model 
      actions[started++] = new SequenceAction(Action::CP(script, xml));
      last_start = script.now;
    }

    // See if we need to wait
    if (!!spread) break;
  }

  // Tick all actions, deleting any that have finished
  for(map<int, Action *>::iterator p = actions.begin(); p!=actions.end();)
  {
    map<int, Action *>::iterator q = p++;  // Protect from deletion
    Action *a = q->second;

    // Set copy variable
    con.vars.add("copy", q->first);

    if (!a->tick(con))
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
  for(map<int, Action *>::iterator p = actions.begin(); p!=actions.end();++p)
    delete p->second;
}


}} // namespaces
