//==========================================================================
// ObTools::XMI: class.cc
//
// UML::Class and UML::Interface functionality
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Constructor
Class::Class(XMI::Reader& rdr, XML::Element& xe)
  :Classifier(rdr, xe)
{
  // Get basic properties
  is_active = get_bool_property("isActive", "UML:Class.isActive");
}

//--------------------------------------------------------------------------
// Printer - adds flags
void Class::print_header(ostream& sout)
{
  GeneralizableElement::print_header(sout);

  if (is_active) sout << " (active)";
}

