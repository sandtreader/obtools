//==========================================================================
// ObTools::XMI: genelem.cc
//
// UML::GeneralizableElement functionality - abstract superclass of all
// inheritable things
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Constructor
GeneralizableElement::GeneralizableElement(XMI::Reader& rdr, XML::Element& xe)
  :ModelElement(rdr, xe)
{
  //Get basic properties
  is_abstract = get_bool_property("isAbstract", 
				  "UML:GeneralizableElement.isAbstract");

  is_root     = get_bool_property("isRoot", 
				  "UML:GeneralizableElement.isRoot");

  is_leaf     = get_bool_property("isLeaf", 
				  "UML:GeneralizableElement.isLeaf");
}

//--------------------------------------------------------------------------
// Printer - adds flags
void GeneralizableElement::print_header(ostream& sout)
{
  ModelElement::print_header(sout);

  if (is_abstract) sout << " (abstract)";
  if (is_root) sout << " (root)";
  if (is_leaf) sout << " (leaf)";
}

