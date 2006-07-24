//==========================================================================
// ObTools::Script: ot-script.h
//
// Public definitions for XML script library
// 
// Copyright (c) 2006 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#ifndef __OBTOOLS_SCRIPT_H
#define __OBTOOLS_SCRIPT_H

#include "ot-xml.h"
#include "ot-init.h"

namespace ObTools { namespace Script {

//Make our lives easier without polluting anyone else
using namespace std;

// Forward
class Language;

//==========================================================================
// Script action (abstract)
// Dynamically created during run - only active actions on the stack will
// be instantiated at any one time 

class Action
{
protected:
  Language& language;     // Language we're a part of
  XML::Element& xml;      // XML element we were created from

public:
  //------------------------------------------------------------------------
  // Constructor
  // Create parameters - have to be bundled for use with Init::Factory
  struct CP
  {
    Language& language;
    XML::Element& xml;
    CP(Language& _language, XML::Element& _xml): 
      language(_language), xml(_xml) {}
  };
  Action(CP cp): language(cp.language), xml(cp.xml) {}

  //------------------------------------------------------------------------
  // Tick action
  // Returns whether still active
  virtual bool tick() = 0;

  //------------------------------------------------------------------------
  // Virtual destructor
  virtual ~Action() {}
};

//==========================================================================
// Single action (abstract)
// Action which does something once then exits
class SingleAction: public Action
{
public:
  //------------------------------------------------------------------------
  // Constructor
  SingleAction(CP cp): Action(cp) {}

  //------------------------------------------------------------------------
  // Tick action
  // Does 'run()', then always returns false
  bool tick() { run(); return false; }

  //------------------------------------------------------------------------
  // Run action
  virtual void run()=0;
};

//==========================================================================
// Sequence action
// Action which executes other actions in sequence (like a language block
// statement)
class SequenceAction: public Action
{
protected:
  list<XML::Element *>::iterator it;
  Action *current;

  // Internals
  bool create_current();

public:
  //------------------------------------------------------------------------
  // Constructor
  SequenceAction(CP cp);

  //------------------------------------------------------------------------
  // Tick action
  // Returns whether still active
  bool tick();

  //------------------------------------------------------------------------
  // Restart sequence
  void restart();

  //------------------------------------------------------------------------
  // Destructor
  virtual ~SequenceAction();
};

//==========================================================================
//Repeat action
// e.g. <repeat times="4">
//        ... sub actions ...
//      </repeat>
class RepeatAction: public SequenceAction
{
private:
  int index;
  int times;

public:
  //------------------------------------------------------------------------
  // Constructor
  RepeatAction(CP cp);

  //------------------------------------------------------------------------
  // Tick action
  // Returns whether still active
  bool tick();
};

//==========================================================================
//Log action
// e.g. <log level="1">Something happened</log>
class LogAction: public SingleAction
{
public:
  //------------------------------------------------------------------------
  // Constructor
  LogAction(Action::CP cp): SingleAction(cp) {}

  //------------------------------------------------------------------------
  // Run action
  void run();
};

//==========================================================================
// Top-level script 
class Script: public SequenceAction
{
public:
  //------------------------------------------------------------------------
  //Constructor - takes language and top-level script element
  Script(Language& language, XML::Element& _xml);

  //------------------------------------------------------------------------
  //Run the script to the end
  void run();
};

//==========================================================================
// General script language class with no set bindings
class Language
{
private:
  // Registry of actions by name
  Init::Registry<Action, Action::CP> action_registry;

protected:
  // Typedef for action factory
  typedef Init::Factory<Action, Action::CP> factory_t;

  //------------------------------------------------------------------------
  //Register a language construct
  void register_action(const string& name, factory_t& factory);

public:
  //------------------------------------------------------------------------
  //Constructor
  Language(): action_registry() {};

  //------------------------------------------------------------------------
  //Instantiate an action from the given XML element
  //Returns 0 if it fails
  Action *create_action(XML::Element& xml);

  //------------------------------------------------------------------------
  //Run the script to the end
  void run();
};

//==========================================================================
// Base script language class with standard bindings:
//  <repeat times="N"/>
class BaseLanguage: public Language
{
public:
  //------------------------------------------------------------------------
  //Constructor
  BaseLanguage();
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_SCRIPT_H



