//==========================================================================
// ObTools::XMI: classifier.cc
//
// UML::Classifier functionality - abstract superclass of Class, Interface
// and DataType
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Constructor
Classifier::Classifier(XMI::Reader& rdr, XML::Element& xe)
  :GeneralizableElement(rdr, xe)
{
  //Read attribute and operation sub-elements from XML source
  read_subelements("UML:Attribute", create_element<Attribute>);
  read_subelements("UML:Operation", create_element<Operation>);
}

//--------------------------------------------------------------------------
// Second-pass reference fix
void Classifier::build_refs()
{
  GeneralizableElement::build_refs();

  //Find association ends
  //!!!
}


