//==========================================================================
// ObTools::Script: language.cc
//
// XML script language definition
//
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-script.h"
#include "ot-log.h"

namespace ObTools { namespace Script {

//------------------------------------------------------------------------
//Register a language construct
void Language::register_action(const string& name, factory_t& factory)
{
  action_registry.add(name, factory);
}

//------------------------------------------------------------------------
//Instantiate an action from the given XML element
//Returns 0 if it fails
Action *Language::create_action(Script& script, XML::Element& xml)
{
  Action *a = action_registry.create(xml.name, Action::CP(script, xml));
  if (a) return a;
  Log::Streams log;
  log.error << "Unknown action '" << xml.name << "' at line " 
	    << xml.line << endl;
  return 0;
}


}} // namespaces
