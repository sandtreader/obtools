//==========================================================================
// ObTools::Init: ot-init.h
//
// Public definitions for ObTools::Init
// Support for auto-initialisation and factories
// 
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#ifndef __OBTOOLS_INIT_H
#define __OBTOOLS_INIT_H

#include <string>
#include <list>
#include <map>
#include <ot-xml.h>

namespace ObTools { namespace Init { 

//Make our lives easier without polluting anyone else
using namespace std;

//==========================================================================
// Initialisation action template
// Abstract superclass representing some initialisation to be done to
// global state, in increasing order of rank
// 
// Suggested values for rank:
//  0 (default)  Independent modules not needing anything else 
//  1            Modules dependent on rank 0 having been initialised
//  2            Modules dependent on rank 1 having been initialised
//  etc. 
//
// Note also that all actions are called after full static initialisation
// of static objects
//
class Action
{
public:
  int rank;

  //------------------------------------------------------------------------
  // Constructor
  Action(int r=0): rank(r) {}

  //------------------------------------------------------------------------
  // Initialise method
  virtual void initialise() = 0;

  //------------------------------------------------------------------------
  // Virtual destructor (does nothing, keeps compiler happy)
  virtual ~Action() {}
};

//==========================================================================
// Initialisation sequence template
// Represents a list of initialisation Actions, to be done in order of rank
//
// NOTE:  Everything is static - a class singleton, if you like
class Sequence
{
public:
  //------------------------------------------------------------------------
  // Register an action
  // This method is designed to work even if it is called before the
  // singleton Sequence is fully initialised
  static void add(Action& a); 

  //------------------------------------------------------------------------
  // Run all initialisations in order
  static void run();
};

//==========================================================================
// Auto-initialising action template
// Still abstract (implement 'initialise'), but automatically registers
// into the sequence
class AutoAction: public Action
{
public:
  //------------------------------------------------------------------------
  // Constructor
  AutoAction(int r=0): Action(r) { Sequence::add(*this); }
};

//==========================================================================
// Factory template
// Abstract superclass for creating objects of superclass <SUPER> with 
// create parameters <CP>.  
// Default create parameter is an XML element (e.g. from config)
template<class SUPER, class CP=const XML::Element&> class Factory
{
private:

public:
  //------------------------------------------------------------------------
  // Constructor
  Factory() {}

  //------------------------------------------------------------------------
  // Create method
  virtual SUPER *create(CP cp) = 0;

  // Virtual destructor to keep compiler happy
  virtual ~Factory() {}
};

//==========================================================================
// NewFactory template
// Factory for objects of type <SUB> with superclass <SUPER> (used in registry)
// with create parameters <CP>, where the create method is simply the
// constructor with parameters <CP>
// Default create parameter is an XML element (e.g. from config)
template<class SUPER, class SUB, class CP=const XML::Element&>
  class NewFactory: public Factory<SUPER, CP>
{
public:
  //------------------------------------------------------------------------
  // Create method
  SUPER *create(CP cp) { return new SUB(cp); }
};

//==========================================================================
// Registry template
// A place to register Factories for superclass <SUPER> by name,
// with create parameters CP
template<class SUPER, class CP=const XML::Element&,
         class KEY=string> class Registry
{
private:
  map<KEY, Factory<SUPER, CP> *> factories;

public:
  //------------------------------------------------------------------------
  // Constructor
  Registry() {}

  //------------------------------------------------------------------------
  // Register a factory by name
  void add(const KEY& name, Factory<SUPER, CP>& f)
  { factories[name] = &f; }

  //------------------------------------------------------------------------
  // Create an object by name
  // Returns the object, or 0 if no factories available or create fails
  SUPER *create(const KEY& name, CP cp)
  {
    typename map<KEY, Factory<SUPER, CP> *>::iterator p;
    p = factories.find(name);
    if (p!=factories.end())
      return p->second->create(cp);
    else
      return 0;
  }
};

//==========================================================================
// AutoRegister template
// Template for automatically registering a NewFactory in the given name
// on the given registry
// <SUPER> is the superclass used in the register
// <SUB> is the subclass being initialised here
template<class SUPER, class SUB, class CP=const XML::Element&,
         class KEY=string>
  class AutoRegister: private AutoAction
{
private:
  NewFactory<SUPER, SUB, CP> factory;
  Registry<SUPER, CP, KEY>& reg;
  KEY name;

public:
  //------------------------------------------------------------------------
  // Constructor
  AutoRegister(Registry<SUPER, CP, KEY>& _reg, const KEY& _name):
    reg(_reg), name(_name) {}

  //------------------------------------------------------------------------
  // Initialiser
  void initialise()
  { reg.add(name, factory); }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_INIT_H
















