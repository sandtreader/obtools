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

  //!!! Get parent/child refs

}

//--------------------------------------------------------------------------
// Printer
void Generalization::print_header(ostream& sout)
{
  ModelElement::print_header(sout);

  //!!! print parent, child
}
