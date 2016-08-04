//==========================================================================
// ObTools::Script: script.cc
//
// Top-level script
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-script.h"
#include "ot-mt.h"

namespace ObTools { namespace Script {

//--------------------------------------------------------------------------
//Constructor - takes top-level script element
Script::Script(Language& _language, XML::Element& _xml):
  SequenceAction(Action::CP(*this, _xml)), language(_language)
{
}

//--------------------------------------------------------------------------
//Tick the script, setting time stamp
//Returns whether it is still running
bool Script::tick()
{
  now = Time::Stamp::now();
  Context con;
  return SequenceAction::tick(con);
}

//--------------------------------------------------------------------------
//Run the script to the end
void Script::run()
{
  while (tick()) this_thread::sleep_for(chrono::milliseconds{10});
}


}} // namespaces
