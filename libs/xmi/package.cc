//==========================================================================
// ObTools::XMI: package.cc
//
// UML::Package and UML::Model functionality
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
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
  const char *p="UML:Package";
  read_subelements("UML:Class",          create_element<Class>,          p);
  read_subelements("UML:DataType",       create_element<DataType>,       p);
  read_subelements("UML:Interface",      create_element<Interface>,      p);
  read_subelements("UML:Stereotype",     create_element<Stereotype>,     p);
  read_subelements("UML:Association",    create_element<Association>,    p);
  read_subelements("UML:Generalization", create_element<Generalization>, p);
  read_subelements("UML:Package",        create_element<Package>,        p);
}

//--------------------------------------------------------------------------
// Model printer - adds version
void Model::print_header(ostream& sout)
{
  GeneralizableElement::print_header(sout);

  if (uml_version) sout << " (UML version " << uml_version << ")";
}


