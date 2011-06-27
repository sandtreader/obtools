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
  Action(cp), race(_race)
{
}

//------------------------------------------------------------------------
// Start action
bool ParallelAction::start(Context& con)
{
  // If not started, start all children actions simultaneously
  for(list<XML::Element *>::iterator p = xml.children.begin();
      p!=xml.children.end(); ++p)
  {
    XML::Element& child = **p;
    Action *a = script.create_action(child);
    if (a)
    {
      a->start(con);
      actions.push_back(a);
    }
  }

  return true;
}

//------------------------------------------------------------------------
// Tick action
// Returns whether still active
bool ParallelAction::tick(Context& con)
{
  // Tick all actions, deleting any that have finished
  for(list<Action *>::iterator p = actions.begin(); p!=actions.end();)
  {
    list<Action *>::iterator q = p++;  // Protect from deletion
    Action *a = *q;
    if (!a->tick(con))
    {
      a->stop(con);
      actions.erase(q);
      delete a;
    }
  }

  // Check for end - race stops if any have finished, normally only when all
  // have finished
  return (race?(actions.size() == xml.children.size()):(!actions.empty()));
}

//------------------------------------------------------------------------
// Stop action
// Stop any still active
void ParallelAction::stop(Context& con)
{
  for(list<Action *>::iterator p = actions.begin(); p!=actions.end();++p)
    (*p)->stop(con);
}

//------------------------------------------------------------------------
// Destructor
ParallelAction::~ParallelAction()
{
  for(list<Action *>::iterator p = actions.begin(); p!=actions.end();++p)
    delete *p;
}


}} // namespaces
