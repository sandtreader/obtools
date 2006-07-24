//==========================================================================
// ObTools::Script: repeat.cc
//
// Repeat action
// <repeat times="N"/> - forever if times not given
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-script.h"

namespace ObTools { namespace Script {

//------------------------------------------------------------------------
// Constructor
RepeatAction::RepeatAction(Action::CP cp): 
  SequenceAction(cp), index(0)
{
  times = xml.get_attr_int("times");
}

//------------------------------------------------------------------------
// Tick action
// Returns whether still active
bool RepeatAction::tick()
{
  // Try to tick sequence - if still running, that's fine
  if (SequenceAction::tick()) return true;

  // Check for number of times exceeded
  if (times && ++index >= times) return false;

  // Try to restart
  restart();
  return true;
}


}} // namespaces
