//==========================================================================
// ObTools::XMI: attribute.cc
//
// UML::Attribute functionality
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Constructor
Attribute::Attribute(XMI::Reader& rdr, XML::Element& xe)
  :StructuralFeature(rdr, xe) 
{
  //!!! Read initial value
}


//--------------------------------------------------------------------------
// Printer - adds initial value
void Attribute::print_header(ostream& sout)
{
  StructuralFeature::print_header(sout);

  //!!! print initial value
}
