//==========================================================================
// ObTools::Init: init.cc
//
// Auto-initialisation support
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-init.h"

namespace ObTools { namespace Init {

//--------------------------------------------------------------------------
// Get the actions list
// This method is designed to work even if it is called before the
// singleton Sequence is fully initialised
static list<Action *> *get_actions()
{
  static list<Action *> *actions = 0;
  if (!actions) actions = new list<Action *>;
  return actions;
}

//--------------------------------------------------------------------------
// Register an action
// This method is designed to work even if it is called before the
// singleton Sequence is fully initialised
void Sequence::add(Action& a)
{
  get_actions()->push_back(&a);
}

//--------------------------------------------------------------------------
// Action pointer comparison function
static bool sort_action_rank(Action *a1, Action *a2)
{
  return a1->rank < a2->rank;
}

//--------------------------------------------------------------------------
// Run all initialisations in order
void Sequence::run()
{
  list<Action *> *actions = get_actions();

  // Sort the list
  actions->sort(sort_action_rank);

  // Run all actions
  for(list<Action *>::iterator p=actions->begin(); p!=actions->end(); p++)
    (*p)->initialise();
}

}} // namespaces
