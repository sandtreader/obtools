//==========================================================================
// ObTools::Init: ot-init.h
//
// Public definitions for ObTools::Init
// Support for auto-initialisation and factories
// 
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
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
// Abstract superclass for creating objects of type <T> with create parameters
// <CP>.  Default create parameter is an XML element (e.g. from config)
template<class T, class CP=XML::Element&> class Factory
{
private:

public:
  //------------------------------------------------------------------------
  // Constructor
  Factory() {}

  //------------------------------------------------------------------------
  // Create method
  virtual T *create(CP cp) = 0;
};

//==========================================================================
// SelfFactory template
// Factory for objects of type <T> with create parameters <CP>, where the
// create method is a static method of the class <T>
// Default create parameter is an XML element (e.g. from config)
template<class T, class CP=XML::Element&> 
  class SelfFactory: public Factory<T, CP>
{
public:
  //------------------------------------------------------------------------
  // Create method
  T *create(CP cp) { return T::create(cp); }
};

//==========================================================================
// Registry template
// A place to register Factories for type <T> by name, with create parameters
// CP
template<class T, class CP=XML::Element&> class Registry
{
private:
  map<string, Factory<T, CP> *> factories;

public:
  //------------------------------------------------------------------------
  // Constructor
  Registry() {}

  //------------------------------------------------------------------------
  // Register a factory by name
  void add(const string& name, Factory<T, CP>& f)
  { factories[name] = &f; }
    
  //------------------------------------------------------------------------
  // Create an object by name
  // Returns the object, or 0 if no factories available or create fails
  T *create(const string& name, CP cp)
  { 
    typename map<string, Factory<T, CP> *>::iterator p = factories.find(name);
    if (p!=factories.end())
      return p->second->create(cp);
    else
      return 0;
  }
};

//==========================================================================
// AutoRegister template
// Template for automatically registering a SelfFactory in the given name
// on the given registry
template<class T, class CP=XML::Element&> 
  class AutoRegister: private AutoAction
{
private:
  SelfFactory<T, CP> factory;
  Registry<T, CP>& reg;
  string name;

public:
  //------------------------------------------------------------------------
  // Constructor
  AutoRegister(Registry<T, CP>& _reg, const string& _name):
    reg(_reg), name(_name) {}

  //------------------------------------------------------------------------
  // Initialiser
  void initialise() { reg.add(name, factory); }
};

//==========================================================================
}} //namespaces
#endif // !__OBTOOLS_INIT_H
















