//==========================================================================
// ObTools::XMI: package.cc
//
// UML::Package and UML::Model functionality
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Constructor
Package::Package(XMI::Reader& rdr, XML::Element& xe)
  :GeneralizableElement(rdr, xe) //Element does the basics
{
  //Read all sub-elements I'm interested in, using their respective
  //factories, pruning at UML:Package to avoid grabbing all
  //sub-package contents as well
  //Note we set id_required to avoid grabbing reference elements for things
  //that can be referenced
  const char *p="UML:Package";
  read_subelements("UML:Class",          create_element<Class>, true, p);
  read_subelements("UML:DataType",       create_element<DataType>, true, p);
  read_subelements("UML:Enumeration",    create_element<Enumeration>, true, p);
  read_subelements("UML:Primitive",      create_element<Primitive>, true, p);
  read_subelements("UML:Interface",      create_element<Interface>, true, p);
  read_subelements("UML:Stereotype",     create_element<Stereotype>, true, p);
  read_subelements("UML:Association",    create_element<Association>, true, p);
  read_subelements("UML:Generalization", create_element<Generalization>, true, 
		                                                           p);
  read_subelements("UML:Package",        create_element<Package>, false, p);
}

//--------------------------------------------------------------------------
// Model printer - adds version
void Model::print_header(ostream& sout)
{
  GeneralizableElement::print_header(sout);

  if (uml_version) sout << " (UML version " << uml_version << ")";
}


