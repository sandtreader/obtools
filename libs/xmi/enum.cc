//==========================================================================
// ObTools::XMI: enumeration.cc
//
// UML::Enumeration functionality
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Constructor
Enumeration::Enumeration(XMI::Reader& rdr, XML::Element& xe)
  :DataType(rdr, xe) 
{
  //!!! Read literals
}

//--------------------------------------------------------------------------
// Printer
void Enumeration::print_header(ostream& sout)
{
  GeneralizableElement::print_header(sout);

  //!!! print literals
}



