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
  :Element(rdr, xe) //Element does the basics
{


}

//--------------------------------------------------------------------------
// Second-pass reference fix
void Attribute::build_refs()
{
  //Get type
  if (!get_type(type))
    reader.error("No type specified for attribute ", name);
}

//--------------------------------------------------------------------------
// Printer
void Attribute::print(ostream& sout, int indent=0)
{
  Element::print(sout, indent);
}

