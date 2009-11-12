//==========================================================================
// ObTools::XMI: general.cc
//
// UML::Generalization functionality
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Second-pass reference fix
void Generalization::build_refs()
{
  ModelElement::build_refs();

  gparent = get_ge_property("parent", "UML:Generalization.parent");
  if (!gparent)
    reader.error("Can't get parent of generalization ", id);

  gchild = get_ge_property("child", "UML:Generalization.child");
  if (!gchild)
    reader.error("Can't get child of generalization ", id);

  //Fix up GeneralizableElement lists of child and parent to point to me
  gchild->generalizations.push_back(this);
  gparent->specializations.push_back(this);
}

//--------------------------------------------------------------------------
// Printer
void Generalization::print_header(ostream& sout)
{
  ModelElement::print_header(sout);

  if (gchild) sout << " " << gchild->name;
  if (gparent) sout << "->" << gparent->name;
}
