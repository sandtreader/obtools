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
  XML::Element& ive = source.get_child("UML:Attribute.initialValue");
  if (ive.valid())
    initial_value = Expression::read_from(ive);
}


//--------------------------------------------------------------------------
// Printer - adds initial value
void Attribute::print_header(ostream& sout)
{
  StructuralFeature::print_header(sout);

  if (!initial_value.body.empty())
    sout << " = '" << initial_value.body << "'";

  if (!initial_value.language.empty())
    sout << " <" << initial_value.language << ">";
}
