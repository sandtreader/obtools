//==========================================================================
// ObTools::Script: ot-script.h
//
// Public definitions for XML script library
// 
// Copyright (c) 2006 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_SCRIPT_H
#define __OBTOOLS_SCRIPT_H

#include "ot-xml.h"
#include "ot-init.h"
#include "ot-misc.h"
#include "ot-time.h"

namespace ObTools { namespace Script {

//Make our lives easier without polluting anyone else
using namespace std;

// Forward
class Script;

//==========================================================================
// Script context
// Represents a scope level
struct Context
{
  Misc::PropertyList vars;

  // Default constructor - empty vars
  Context() {}

  // Copy constructor
  Context(const Context& o): vars(o.vars) {}
};

//==========================================================================
// Script action (abstract)
// Dynamically created during run - only active actions on the stack will
// be instantiated at any one time 
class Action
{
protected:
  Script& script;         // Top-level script we're a part of
  XML::Element& xml;      // XML element we were created from

public:
  //------------------------------------------------------------------------
  // Constructor
  // Create parameters - have to be bundled for use with Init::Factory
  struct CP
  {
    Script& script;
    XML::Element& xml;
    CP(Script& _script, XML::Element& _xml): script(_script), xml(_xml) {}
  };
  Action(CP cp): script(cp.script), xml(cp.xml) {}

  //------------------------------------------------------------------------
  // Tick action
  // Returns whether still active
  virtual bool tick(Context&) = 0;

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
  bool tick(Context& con) { run(con); return false; }

  //------------------------------------------------------------------------
  // Run action
  virtual void run(Context& con)=0;
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
  virtual bool tick(Context& con);

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
  bool tick(Context& con);
};

//==========================================================================
// Parallel action
// Action which executes other actions in parallel (like PAR in Occam)
class ParallelAction: public Action
{
private:
  bool race;
  bool started;
  list<Action *> actions;

public:
  //------------------------------------------------------------------------
  // Constructor
  // If 'race' is set, the entire group is stopped when the first one 
  // finishes; otherwise, the group continues until the last finishes
  ParallelAction(CP cp, bool race);

  //------------------------------------------------------------------------
  // Tick action
  // Returns whether still active
  bool tick(Context& con);

  //------------------------------------------------------------------------
  // Destructor
  virtual ~ParallelAction();
};

//==========================================================================
// Group action
// Sugar for ParallelAction with race semantics
class GroupAction: public ParallelAction
{
public:
  //------------------------------------------------------------------------
  // Constructor
  GroupAction(CP cp): ParallelAction(cp, false) {}
};

//==========================================================================
// Race action
// Sugar for ParallelAction with race semantics
class RaceAction: public ParallelAction
{
public:
  //------------------------------------------------------------------------
  // Constructor
  RaceAction(CP cp): ParallelAction(cp, true) {}
};

//==========================================================================
// Replicated action
// Action which runs N copies of the same sequence
// <replicate copies="5" spread="1"/>
class ReplicatedAction: public Action
{
private:
  int copies;
  Time::Duration spread;
  int started;
  Time::Stamp last_start;
  map<int, Action *> actions;

public:
  //------------------------------------------------------------------------
  // Constructor
  ReplicatedAction(CP cp);

  //------------------------------------------------------------------------
  // Tick action
  // Returns whether still active
  bool tick(Context& con);

  //------------------------------------------------------------------------
  // Destructor
  virtual ~ReplicatedAction();
};

//==========================================================================
// Scope action
// Action which provides a new scope level.  Note variables from outer levels
// are copied into the new scope, not referred - therefore updates are not
// passed out
class ScopeAction: public SequenceAction
{
private:
  Context context;  // Our new context
  bool ticked;

public:
  //------------------------------------------------------------------------
  // Constructor
  ScopeAction(CP cp);

  //------------------------------------------------------------------------
  // Tick action
  // Returns whether still active
  bool tick(Context& con);
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
  void run(Context& con);
};

//==========================================================================
//Delay action
// e.g. <delay time="1" random="yes"/>
class DelayAction: public Action
{
  Time::Stamp start;
  Time::Duration time;

public:
  //------------------------------------------------------------------------
  // Constructor
  DelayAction(Action::CP cp);

  //------------------------------------------------------------------------
  // Tick action
  // Returns whether still active
  bool tick(Context& con);
};

//==========================================================================
//Set action
// e.g. <set var="foo">content</set>
class SetAction: public SingleAction
{
public:
  //------------------------------------------------------------------------
  // Constructor
  SetAction(Action::CP cp): SingleAction(cp) {}

  //------------------------------------------------------------------------
  // Run action
  void run(Context& con);
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
  //Instantiate an action from the given script and XML element
  //Returns 0 if it fails
  Action *create_action(Script& script, XML::Element& xml);

  //------------------------------------------------------------------------
  //Run the script to the end
  void run();
};

//==========================================================================
// Base script language class with standard bindings:
//  <sequence>...</sequence>
//  <repeat times="N">...</repeat>
//  <group>...</group>
//  <race>...</race>
//  <replicate copies="N" spread="T">...</replicate>
//  <delay time="N" random="yes"/>
//  <log level="N">text</log>
//
class BaseLanguage: public Language
{
public:
  //------------------------------------------------------------------------
  //Constructor
  BaseLanguage();
};

//==========================================================================
// Top-level script 
class Script: public SequenceAction
{
public:
  Language& language;       // Language in use
  Misc::PropertyList vars;  // Global variables for script actions
  Time::Stamp now;          // Consistent time for ticks

  //------------------------------------------------------------------------
  //Constructor - takes language and top-level <script> XML element
  Script(Language& _language, XML::Element& _xml);

  //------------------------------------------------------------------------
  //Instantiate an action from the given XML element
  //Returns 0 if it fails
  Action *create_action(XML::Element& xml)
  { return language.create_action(*this, xml); }

  //------------------------------------------------------------------------
  //Tick the script, setting time stamp
  //Returns whether it is still running
  bool tick();

  //------------------------------------------------------------------------
  //Run the script to the end
  void run();
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_SCRIPT_H



