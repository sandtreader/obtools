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
Script::Script(Language& language, XML::Element& _xml): 
  SequenceAction(Action::CP(language, _xml))
{
}

//------------------------------------------------------------------------
//Run the script to the end
void Script::run()
{
  while (tick()) usleep(10000);
}


}} // namespaces
