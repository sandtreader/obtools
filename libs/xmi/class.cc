//==========================================================================
// ObTools::XMI: class.cc
//
// UML::Class functionality
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Constructor
Class::Class(XMI::Reader& rdr, XML::Element& xe)
  :Element(rdr, xe),  //Element does the basics
   stereotype(0)   
{
  //Get basic properties
  is_abstract = get_bool_property("isAbstract", 
				  "UML:GeneralizableElement.isAbstract");

  //Read attributes
  OBTOOLS_XML_FOREACH_DESCENDANT_WITH_TAG(ae, source, "UML:Attribute")
    UML::Attribute *a = new Attribute(rdr, ae);
    elements.push_back(a);
    attributes.push_back(a);
  OBTOOLS_XML_ENDFOR
}

//--------------------------------------------------------------------------
// Second-pass reference fix
void Class::build_refs()
{
  //Get stereotype ref

  //Fix children (attributes & operations)
  Element::build_refs();
}

//--------------------------------------------------------------------------
// Printer
void Class::print(ostream& sout, int indent=0)
{
  sout << string(indent, ' ');
  sout << "Class '" << name << "'";

  if (!id.empty())
    sout << " (" << id << ")";

  if (is_abstract) sout << " (abstract)";
  if (stereotype) sout << " <<" << stereotype->name << ">>";

  sout << endl;

  //List subelements (if any)
  print_subelements(sout, indent+2);
}

