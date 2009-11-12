//==========================================================================
// ObTools::Script: baselang.cc
//
// Basic language definition
//
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-script.h"

namespace ObTools { namespace Script {

//------------------------------------------------------------------------
//Factories for standard controls

static Init::NewFactory<Action, SequenceAction, Action::CP> sequence_factory;
static Init::NewFactory<Action, RepeatAction,   Action::CP> repeat_factory;
static Init::NewFactory<Action, GroupAction,    Action::CP> group_factory;
static Init::NewFactory<Action, RaceAction,     Action::CP> race_factory;
static Init::NewFactory<Action, ReplicatedAction, 
			Action::CP> replicated_factory;
static Init::NewFactory<Action, LogAction,      Action::CP> log_factory;
static Init::NewFactory<Action, DelayAction,    Action::CP> delay_factory;

//------------------------------------------------------------------------
//Constructor
BaseLanguage::BaseLanguage(): Language()
{
  register_action("sequence", sequence_factory);
  register_action("repeat", repeat_factory);
  register_action("group",  group_factory);
  register_action("race",   race_factory);
  register_action("replicate", replicated_factory);
  register_action("log",    log_factory);
  register_action("delay",  delay_factory);
}


}} // namespaces
