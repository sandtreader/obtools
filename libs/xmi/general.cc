//==========================================================================
// ObTools::XMI: general.cc
//
// UML::Generalization functionality
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Second-pass reference fix
void Generalization::build_refs()
{
  ModelElement::build_refs();

  parent = get_ge_property("parent", "UML:Generalization.parent");
  if (!parent)
    reader.error("Can't get parent of generalization ", id);

  child = get_ge_property("child", "UML:Generalization.child");
  if (!child)
    reader.error("Can't get child of generalization ", id);

  //Fix up GeneralizableElement lists of child and parent to point to me
  child->generalizations.push_back(this);
  parent->specializations.push_back(this);
}

//--------------------------------------------------------------------------
// Printer
void Generalization::print_header(ostream& sout)
{
  ModelElement::print_header(sout);

  if (child) sout << " " << child->name;
  if (parent) sout << "->" << parent->name;
}
