//==========================================================================
// ObTools::Script: delay.cc
//
// Delay action
//   <delay time="N" random="yes/no"/>
//   time:    Time to wait in seconds or duration (default, one tick)
//   random:  Randomise delay up to 'time' (default no)
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-script.h"
#include "ot-log.h"
#include <stdlib.h>

namespace ObTools { namespace Script {

//--------------------------------------------------------------------------
// Constructor
DelayAction::DelayAction(const CP& cp): Action(cp)
{
  start = script.now;
  time = Time::Duration(xml.get_attr("time","0"));

  // Check for randomisation
  if (xml.get_attr_bool("random") && !!time)
    time = Time::Duration(static_cast<double>(
                              rand() % static_cast<int>(time.seconds())));
}


//--------------------------------------------------------------------------
// Tick action
// Returns whether still active
bool DelayAction::tick(Context&)
{
  return script.now<=start || script.now-start < time;
}


}} // namespaces
