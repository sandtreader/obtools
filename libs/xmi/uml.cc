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

    case CLASS_DATATYPE:
      sout << " datatype";
      break;
  }

  if (!stereotype.empty())
    sout << " <" << stereotype << ">";

  sout << ":" << endl;
  print_subelements(sout, indent+2);
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
  sout << "Attribute '" << name << ":" << type->name << "'" << endl;
}


