//==========================================================================
// ObTools::Script: parallel.cc
//
// Parallel action
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-script.h"

namespace ObTools { namespace Script {

//------------------------------------------------------------------------
// Constructor
ParallelAction::ParallelAction(const CP& cp, bool _race): 
  Action(cp), race(_race), started(false)
{
}

//------------------------------------------------------------------------
// Tick action
// Returns whether still active
bool ParallelAction::tick(Context& con)
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
    if (!a->tick(con))
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
