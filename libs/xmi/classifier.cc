//==========================================================================
// ObTools::XMI: classifier.cc
//
// UML::Classifier functionality - abstract superclass of Class, Interface
// and DataType
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Constructor
Classifier::Classifier(XMI::Reader& rdr, XML::Element& xe)
  :GeneralizableElement(rdr, xe)
{
  //Read attribute and operation sub-elements from XML source
  //They must have xmi.id's, otherwise they may be references
  read_subelements("UML:Attribute", create_element<Attribute>, true);
  read_subelements("UML:Operation", create_element<Operation>, true);

  //Log me in the reader's classmap for instant access
  reader.class_map[name] = this;
}



