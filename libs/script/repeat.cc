//==========================================================================
// ObTools::Script: repeat.cc
//
// Repeat action
// <repeat times="N"/> - forever if times not given
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-script.h"

namespace ObTools { namespace Script {

//------------------------------------------------------------------------
// Constructor
RepeatAction::RepeatAction(const CP& cp):
  SequenceAction(cp), index(0)
{
  times = xml.get_attr_int("times");
}

//------------------------------------------------------------------------
// Tick action
// Returns whether still active
bool RepeatAction::tick(Context& con)
{
  // Set scope variable
  con.vars.add("index", index);

  // Try to tick sequence - if still running, that's fine
  if (SequenceAction::tick(con)) return true;

  // Check for number of times exceeded
  if (times && ++index >= times) return false;

  // Try to restart
  restart();
  return true;
}


}} // namespaces
