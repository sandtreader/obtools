//==========================================================================
// ObTools::XMI: class.cc
//
// UML::Feature, UML::StructuralFeature and UML::BehaviouralFeature
// functionality
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//==========================================================================
// UML::Feature class

//--------------------------------------------------------------------------
// Constructor
Feature::Feature(XMI::Reader& rdr, XML::Element& xe)
  :ModelElement(rdr, xe)
{
  // Get basic properties
  is_static = (get_property("ownerScope", "UML:Feature.ownerScope")
               == "classifier");
}

//--------------------------------------------------------------------------
// Printer - adds flags
void Feature::print_header(ostream& sout)
{
  ModelElement::print_header(sout);

  if (is_static) sout << " (static)";
}

//==========================================================================
// UML::StructuralFeature class

StructuralFeature::StructuralFeature(XMI::Reader& rdr, XML::Element& xe):
  Feature(rdr, xe), type(0)
{
  // Get basic properties
  is_ordered = (get_property("ordering", "UML:StructuralFeature.ordering")
                == "ordered");

  multiplicity = Multiplicity::read_from(source, reader);
}

//--------------------------------------------------------------------------
// Second-pass reference fix
void StructuralFeature::build_refs()
{
  ModelElement::build_refs();

  type = get_classifier_property("type", "UML:StructuralFeature.type");
  if (!type)
    reader.error("Can't get type of attribute ", id);
}

//--------------------------------------------------------------------------
// Printer - adds flags
void StructuralFeature::print_header(ostream& sout)
{
  Feature::print_header(sout);

  if (is_ordered) sout << " (ordered)";

  if (type)
    sout << " " << type->name;

  sout << multiplicity;
}

//==========================================================================
// UML::BehaviouralFeature class

BehaviouralFeature::BehaviouralFeature(XMI::Reader& rdr, XML::Element& xe)
  :Feature(rdr, xe)
{
  // Get basic properties
  is_query = get_bool_property("isQuery", "UML:BehaviouralFeature.isQuery");
}

//--------------------------------------------------------------------------
// Printer - adds flags
void BehaviouralFeature::print_header(ostream& sout)
{
  Feature::print_header(sout);

  if (is_query) sout << " (query)";
}

//--------------------------------------------------------------------------
// Sugar function to 'return' pseudo-parameter - only takes first
// Returns 0 if no return (void)
Parameter *BehaviouralFeature::get_return()
{
  for(list<Element *>::iterator p=subelements.begin();
      p!=subelements.end();
      p++)
  {
    Parameter *par = dynamic_cast<Parameter *>(*p);
    if (par && par->kind == PARAMETER_RETURN) return par;
  }

  return 0;
}

//--------------------------------------------------------------------------
// Sugar function to get parameters - also removes 'return' pseudo-parameter
list<Parameter *> BehaviouralFeature::get_parameters()
{
  list<Parameter *>l;
  for(list<Element *>::iterator p=subelements.begin();
      p!=subelements.end();
      p++)
  {
    Parameter *par = dynamic_cast<Parameter *>(*p);
    if (par && par->kind != PARAMETER_RETURN) l.push_back(par);
  }

  return l;
}


