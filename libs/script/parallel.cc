//==========================================================================
// ObTools::Script: parallel.cc
//
// Parallel action
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-script.h"

namespace ObTools { namespace Script {

//------------------------------------------------------------------------
// Constructor
ParallelAction::ParallelAction(Action::CP cp, bool _race): 
  Action(cp), race(_race), started(false)
{
}

//------------------------------------------------------------------------
// Tick action
// Returns whether still active
bool ParallelAction::tick()
{
  // If not started, start all children actions simultaneously
  if (!started)
  {
    for(list<XML::Element *>::iterator p = xml.children.begin();
	p!=xml.children.end(); ++p)
    {
      XML::Element& child = **p;
      Action *a = script.create_action(child);
      if (a) actions.push_back(a);
    }

    started = true;
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

  // Check for end - race stops if any have finished, normally only when all
  // have finished
  return (race?(actions.size() == xml.children.size()):(!actions.empty()));
}

//------------------------------------------------------------------------
// Destructor
ParallelAction::~ParallelAction()
{
  for(list<Action *>::iterator p = actions.begin(); p!=actions.end();++p)
    delete *p;
}


}} // namespaces
