//==========================================================================
// ObTools::XMI: attribute.cc
//
// UML::Attribute functionality
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Constructor
Attribute::Attribute(XMI::Reader& rdr, XML::Element& xe)
  :StructuralFeature(rdr, xe) 
{
  XML::Element& ive = source.get_child("UML:Attribute.initialValue");
  if (ive.valid())
  {
    // Look for UML:expression subelement
    // !XMI: Don't yet handle all the myriad subtypes
    XML::Element& expe = ive.get_child("UML:Expression");
    if (expe.valid())
    {
      // Read from either 'language' attribute or equivalent subelement
      if (expe.has_attr("language"))
	initial_value.language = expe.get_attr("language");
      else
      {
	XML::Element lange = expe.get_child("UML:Expression.language");
	if (lange.valid()) 
	  initial_value.language = lange.content;
      }

      // Ditto for 'body'
      if (expe.has_attr("body"))
	initial_value.body = expe.get_attr("body");
      else
      {
	XML::Element bodye = expe.get_child("UML:Expression.body");
	if (bodye.valid()) 
	  initial_value.body = bodye.content;
      }
    }
  }
}


//--------------------------------------------------------------------------
// Printer - adds initial value
void Attribute::print_header(ostream& sout)
{
  StructuralFeature::print_header(sout);

  if (!initial_value.body.empty())
    sout << " = '" << initial_value.body << "'";

  if (!initial_value.language.empty())
    sout << " <" << initial_value.language << ">";
}
