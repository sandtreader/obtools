//==========================================================================
// ObTools::Script: random.cc
//
// Randomly perform action at a given probability
// <random probability="0.001"/>
//   .. sequence of actions ..
// </random>
//
// Copyright (c) 2014 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-script.h"

namespace ObTools { namespace Script {

//--------------------------------------------------------------------------
// Constructor
RandomAction::RandomAction(const CP& cp):
  SequenceAction(cp)
{
  double probability = xml.get_attr_real("probability");

  running = rand() < static_cast<int>(probability * RAND_MAX);
}

//--------------------------------------------------------------------------
// Tick action
// Returns whether still active
bool RandomAction::tick(Context& con)
{
  if (!running) return false;

  // Tick sequence to end
  return SequenceAction::tick(con);
}


}} // namespaces
