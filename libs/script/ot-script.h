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

// Make our lives easier without polluting anyone else
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

  // Copy assignment
  Context& operator=(const Context& o) { vars = o.vars; return *this; }
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
  Action(const CP& cp): script(cp.script), xml(cp.xml) {}

  //------------------------------------------------------------------------
  // Start action - when first created.  Does nothing by default.
  // Returns whether runnable
  virtual bool start(Context&) { return true; }

  //------------------------------------------------------------------------
  // Tick action
  // Returns whether still active
  virtual bool tick(Context&) = 0;

  //------------------------------------------------------------------------
  // Stop action - when finished or being killed.  Does nothing by default.
  virtual void stop(Context&) {}

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
  SingleAction(const CP& cp): Action(cp) {}

  //------------------------------------------------------------------------
  // Start action
  // Does 'run()'
  bool start(Context& con) { return run(con); }

  //------------------------------------------------------------------------
  // Tick action
  // Always returns false
  bool tick(Context&) { return false; }

  //------------------------------------------------------------------------
  // Run action - return false if you don't want the script to continue
  virtual bool run(Context& con)=0;
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
  SequenceAction(const CP& cp);

  //------------------------------------------------------------------------
  // Tick action
  // Returns whether still active
  bool tick(Context& con) override;

  //------------------------------------------------------------------------
  // Stop action - when finished or being killed
  void stop(Context&) override;

  //------------------------------------------------------------------------
  // Restart sequence
  void restart();

  //------------------------------------------------------------------------
  // Destructor
  virtual ~SequenceAction();
};

//==========================================================================
// Repeat action
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
  RepeatAction(const CP& cp);

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
  list<Action *> actions;

public:
  //------------------------------------------------------------------------
  // Constructor
  // If 'race' is set, the entire group is stopped when the first one
  // finishes; otherwise, the group continues until the last finishes
  ParallelAction(const CP& cp, bool race);

  //------------------------------------------------------------------------
  // Start action
  bool start(Context& con);

  //------------------------------------------------------------------------
  // Tick action
  // Returns whether still active
  bool tick(Context& con);

  //------------------------------------------------------------------------
  // Stop action
  // Stop any still active
  void stop(Context& con);

  //------------------------------------------------------------------------
  // Destructor
  virtual ~ParallelAction();
};

//==========================================================================
// Group action
// Sugar for ParallelAction with non-race semantics
class GroupAction: public ParallelAction
{
public:
  //------------------------------------------------------------------------
  // Constructor
  GroupAction(const CP& cp): ParallelAction(cp, false) {}
};

//==========================================================================
// Race action
// Sugar for ParallelAction with race semantics
class RaceAction: public ParallelAction
{
public:
  //------------------------------------------------------------------------
  // Constructor
  RaceAction(const CP& cp): ParallelAction(cp, true) {}
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
  ReplicatedAction(const CP& cp);

  //------------------------------------------------------------------------
  // Tick action
  // Returns whether still active
  bool tick(Context& con);

  //------------------------------------------------------------------------
  // Stop action
  // Stop any still active
  void stop(Context& con);

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
  ScopeAction(const CP& cp);

  //------------------------------------------------------------------------
  // Tick action
  // Returns whether still active
  bool tick(Context& con);
};

//==========================================================================
// Thread action
// Action which runs the sequence within it in a separate processor thread
// - use for blocking sub-actions such as network connections etc.
// Note:  Creates its own context
class ThreadAction: public SequenceAction
{
private:
  class ActionThread: public MT::Thread
  {
    ThreadAction& action;
    Context context;
    virtual void run() { action.run_thread(context); }

  public:
    ActionThread(ThreadAction& a, Context& c): action(a), context(c)
      { start(); }
  };

  ActionThread *thread;
  int sleep_time;

public:
  //------------------------------------------------------------------------
  // Constructor
  ThreadAction(const CP& cp);

  //------------------------------------------------------------------------
  // Tick action
  // Returns whether still active
  bool tick(Context& con);

  //------------------------------------------------------------------------
  // Run thread - called from ActionThread
  void run_thread(Context& con);

  //------------------------------------------------------------------------
  // Destructor
  virtual ~ThreadAction();
};

//==========================================================================
// Log action
// e.g. <log level="1">Something happened</log>
class LogAction: public SingleAction
{
public:
  //------------------------------------------------------------------------
  // Constructor
  LogAction(const CP& cp): SingleAction(cp) {}

  //------------------------------------------------------------------------
  // Run action
  bool run(Context& con);
};

//==========================================================================
// Delay action
// e.g. <delay time="1" random="yes"/>
class DelayAction: public Action
{
  Time::Stamp start;
  Time::Duration time;

public:
  //------------------------------------------------------------------------
  // Constructor
  DelayAction(const CP& cp);

  //------------------------------------------------------------------------
  // Tick action
  // Returns whether still active
  bool tick(Context& con);
};

//==========================================================================
// Set action
// e.g. <set var="foo">content</set>
class SetAction: public SingleAction
{
public:
  //------------------------------------------------------------------------
  // Constructor
  SetAction(const CP& cp): SingleAction(cp) {}

  //------------------------------------------------------------------------
  // Run action
  bool run(Context& con);
};

//==========================================================================
// Random action
// e.g. <random probability="0.01">
//        ... sub actions executed only at given probability ...
//      </random>
class RandomAction: public SequenceAction
{
private:
  bool running;

public:
  //------------------------------------------------------------------------
  // Constructor
  RandomAction(const CP& cp);

  //------------------------------------------------------------------------
  // Tick action
  // Returns whether still active
  bool tick(Context& con);
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
  // Register a language construct
  void register_action(const string& name, factory_t& factory);

public:
  //------------------------------------------------------------------------
  // Constructor
  Language(): action_registry() {};

  //------------------------------------------------------------------------
  // Instantiate an action from the given script and XML element
  // Returns 0 if it fails
  Action *create_action(Script& script, XML::Element& xml);

  //------------------------------------------------------------------------
  // Run the script to the end
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
  // Constructor
  BaseLanguage();
};

//==========================================================================
// Top-level script
class Script: public SequenceAction
{
private:
  //------------------------------------------------------------------------
  // Bring in tick from base class
  using SequenceAction::tick;

public:
  Language& language;       // Language in use
  Misc::PropertyList vars;  // Global variables for script actions
  Time::Stamp now;          // Consistent time for ticks

  //------------------------------------------------------------------------
  // Constructor - takes language and top-level <script> XML element
  Script(Language& _language, XML::Element& _xml);

  //------------------------------------------------------------------------
  // Instantiate an action from the given XML element
  // Returns 0 if it fails
  Action *create_action(XML::Element& xml)
  { return language.create_action(*this, xml); }

  //------------------------------------------------------------------------
  // Tick the script, setting time stamp
  // Returns whether it is still running
  bool tick();

  //------------------------------------------------------------------------
  // Run the script to the end
  void run();
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_SCRIPT_H



