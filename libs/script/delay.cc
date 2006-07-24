//==========================================================================
// ObTools::Script: delay.cc
//
// Delay action
//   <delay time="N" random="yes/no"/>
//   time:    Time to wait in seconds or duration (default, one tick)
//   random:  Randomise delay up to 'time' (default no)
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-script.h"
#include "ot-log.h"
#include <stdlib.h>

namespace ObTools { namespace Script {

//------------------------------------------------------------------------
// Constructor
DelayAction::DelayAction(Action::CP cp): Action(cp)
{
  start = script.now;
  time = Time::Duration(xml.get_attr("time","0"));
  
  // Check for randomisation
  if (xml.get_attr_bool("random") && !!time)
    time = Time::Duration((double)(random() % (int)time.seconds())); 
}


//------------------------------------------------------------------------
// Tick action
// Returns whether still active
bool DelayAction::tick()
{
  return script.now<=start || script.now-start < time;
}


}} // namespaces
