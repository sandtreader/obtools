//==========================================================================
// ObTools::XMI: assocend.cc
//
// UML::AssociationEnd functionality
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Constructor
AssociationEnd::AssociationEnd(XMI::Reader& rdr, XML::Element& xe)
  :Element(rdr, xe) //Element does the basics
{


}

//--------------------------------------------------------------------------
// Printer
void AssociationEnd::print(ostream& sout, int indent=0)
{
  Element::print(sout, indent);
}

