//==========================================================================
// ObTools::XMI: elems.cc
//
// XMI element classes
// Not much in here except destructors and debug!
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "xmi.h"
using namespace ObTools::XMI;

//--------------------------------------------------------------------------
// Package destructor
Package::~Package()
{
  //Delete classes
  for(list<Class *>::iterator p=classes.begin();
      p!=classes.end();
      p++)
    delete *p;

  //Delete associations
  for(list<Association *>::iterator p=associations.begin();
      p!=associations.end();
      p++)
    delete *p;

  //Delete sub-packages
  for(list<Package *>::iterator p=packages.begin();
      p!=packages.end();
      p++)
    delete *p;
}


