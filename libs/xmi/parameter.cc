//==========================================================================
// ObTools::XMI: parameter.cc
//
// UML::Parameter functionality
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Constructor
Parameter::Parameter(XMI::Reader& rdr, XML::Element& xe)
  :Element(rdr, xe) //Element does the basics
{


}

//--------------------------------------------------------------------------
// Printer
void Parameter::print(ostream& sout, int indent=0)
{
  Element::print(sout, indent);
}

