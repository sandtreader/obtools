//==========================================================================
// ObTools::XMI: uml.cc
//
// UML model classes
// Not much interesting in here - just destructors and debug streaming
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

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

//--------------------------------------------------------------------------
// Package printer
void Package::print(ostream& sout, int indent)
{
  sout << string(indent, ' ');
  sout << "Package '" << name << "':" << endl;

  //Print sub-packages
  for(list<Package *>::iterator p=packages.begin();
      p!=packages.end();
      p++)
    (*p)->print(sout, indent+2);

  //Print classes
  for(list<Class *>::iterator p=classes.begin();
      p!=classes.end();
      p++)
    (*p)->print(sout, indent+2);

  //Print associations
  for(list<Association *>::iterator p=associations.begin();
      p!=associations.end();
      p++)
    (*p)->print(sout, indent+2);
}

//--------------------------------------------------------------------------
// Class destructor
Class::~Class()
{
  //Delete attributes
  for(list<Attribute *>::iterator p=attributes.begin();
      p!=attributes.end();
      p++)
    delete *p;

  //Delete operations
  for(list<Operation *>::iterator p=operations.begin();
      p!=operations.end();
      p++)
    delete *p;
}

//--------------------------------------------------------------------------
// Class printer
void Class::print(ostream& sout, int indent)
{
  sout << string(indent, ' ');
  sout << "Class '" << name << "'";

  switch (kind)
  {
    case CLASS_CONCRETE:
      break;

    case CLASS_ABSTRACT:
      sout << " abstract";
      break;

    case CLASS_PRIMITIVE:
      sout << " primitive";
      break;
  }

  if (!stereotype.empty())
    sout << " <" << stereotype << ">";

  sout << ":" << endl;

  //Print attributes
  for(list<Attribute *>::iterator p=attributes.begin();
      p!=attributes.end();
      p++)
    (*p)->print(sout, indent+2);

  //Print operations
  for(list<Operation *>::iterator p=operations.begin();
      p!=operations.end();
      p++)
    (*p)->print(sout, indent+2);
}

//--------------------------------------------------------------------------
// Association printer
void Association::print(ostream& sout, int indent)
{
  sout << string(indent, ' ');
  sout << "Association";
  if (!name.empty())
    sout << "'" << name << "'";
  sout << endl;
}

//--------------------------------------------------------------------------
// Operation destructor
Operation::~Operation()
{
  //Delete parameters
  for(list<Parameter *>::iterator p=parameters.begin();
      p!=parameters.end();
      p++)
    delete *p;
}

//--------------------------------------------------------------------------
// Operation printer
void Operation::print(ostream& sout, int indent)
{
  sout << string(indent, ' ');
  sout << "Operation '" << name << "'" << endl;
}

//--------------------------------------------------------------------------
// Attribute printer
void Attribute::print(ostream& sout, int indent)
{
  sout << string(indent, ' ');
  sout << "Attribute '" << name << "'" << endl;
}


