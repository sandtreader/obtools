//==========================================================================
// ObTools::Script: scope.cc
//
// Scope action - introduces a new context level
//
// <scope>
//   .. sequence ..
// </scope>
//
// Copyright (c) 2011 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-script.h"

namespace ObTools { namespace Script {

//------------------------------------------------------------------------
// Constructor
ScopeAction::ScopeAction(const CP& cp):
  SequenceAction(cp), ticked(false)
{

}

//------------------------------------------------------------------------
// Tick action
// Returns whether still active
bool ScopeAction::tick(Context& con)
{
  // If the first time, capture the outer context
  if (!ticked)
  {
    context = con;
    ticked = true;
  }

  // Tick sequence with our own context
  return SequenceAction::tick(context);
}


}} // namespaces
