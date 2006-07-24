//==========================================================================
// ObTools::Script: sequence.cc
//
// Sequence action
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-script.h"

namespace ObTools { namespace Script {

//------------------------------------------------------------------------
// Constructor
SequenceAction::SequenceAction(Action::CP cp): 
  Action(cp), it(xml.children.begin()), current(0)
{
}

//------------------------------------------------------------------------
// Create new current action
// Returns whether created
bool SequenceAction::create_current()
{
  if (it == xml.children.end())
  {
    current = 0;
    return false;
  }

  current = language.create_action(**it);
  return (current != 0);
}

//------------------------------------------------------------------------
// Tick action
// Returns whether still active
bool SequenceAction::tick()
{
  if (!current)
  {
    // Try to create it, and move iterator to next
    if (!create_current()) return false;
    ++it;
  }

  // Tick current
  if (current->tick()) return true;

  // If it's finished, delete it and mark for next time
  delete current;
  current = 0;

  // Only ask to continue if there are any more
  return (it != xml.children.end());
}

//------------------------------------------------------------------------
// Restart sequence
void SequenceAction::restart()
{
  it = xml.children.begin();
  if (current)
  {
    delete current;
    current = 0;
  }
}

//------------------------------------------------------------------------
// Destructor
SequenceAction::~SequenceAction()
{
  if (current) delete current;
}


}} // namespaces
