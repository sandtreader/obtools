//==========================================================================
// ObTools::Script: script.cc
//
// Top-level script
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-script.h"

namespace ObTools { namespace Script {

//------------------------------------------------------------------------
//Constructor - takes top-level script element
Script::Script(Language& _language, XML::Element& _xml): 
  SequenceAction(Action::CP(*this, _xml)), language(_language)
{
}

//------------------------------------------------------------------------
//Tick the script, setting time stamp
//Returns whether it is still running
bool Script::tick()
{
  now = Time::Stamp::now();
  return SequenceAction::tick();
}

//------------------------------------------------------------------------
//Run the script to the end
void Script::run()
{
  while (tick()) usleep(10000);
}


}} // namespaces
