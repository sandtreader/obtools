//==========================================================================
// ObTools::Script: baselang.cc
//
// Basic language definition
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-script.h"

namespace ObTools { namespace Script {

//------------------------------------------------------------------------
//Factories for standard controls

// Repeat
static Init::NewFactory<Action, RepeatAction, Action::CP> repeat_factory;

// Log
static Init::NewFactory<Action, LogAction, Action::CP> log_factory;

//------------------------------------------------------------------------
//Constructor
BaseLanguage::BaseLanguage(): Language()
{
  register_action("repeat", repeat_factory);
  register_action("log", log_factory);
}


}} // namespaces
