//==========================================================================
// ObTools::XMI: parameter.cc
//
// UML::Parameter functionality
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Constructor
Parameter::Parameter(XMI::Reader& rdr, XML::Element& xe)
  :ModelElement(rdr, xe), type(0) 
{
  //Get basic properties
  string pdk  = get_property("kind", "UML:Parameter.kind");

  if (pdk.empty() || pdk=="in")
    kind=PARAMETER_IN;
  else if (pdk=="inout")
    kind=PARAMETER_INOUT;
  else if (pdk=="out")
    kind=PARAMETER_OUT;
  else if (pdk=="return")
    kind=PARAMETER_RETURN;
  else
  {
    reader.warning("Unknown parameter kind: ", pdk);
    kind=PARAMETER_IN;  // Safest
  }

  XML::Element& dve = source.get_child("UML:Attribute.defaultValue");
  if (dve.valid())
    default_value = Expression::read_from(dve);
}

//--------------------------------------------------------------------------
// Second-pass reference fix
void Parameter::build_refs()
{
  ModelElement::build_refs();

  type = get_classifier_property("type", "UML:Parameter.type");
  if (!type)
    reader.error("Can't get type of parameter ", id);
}

//--------------------------------------------------------------------------
// Printer
void Parameter::print_header(ostream& sout)
{
  ModelElement::print_header(sout);

  if (type)
    sout << " " << type->name;

  if (!default_value.body.empty())
    sout << " = '" << default_value.body << "'";

  if (!default_value.language.empty())
    sout << " <" << default_value.language << ">";

  switch (kind)
  {
    case PARAMETER_IN:
      sout << " (in)";
      break;

    case PARAMETER_INOUT:
      sout << " (inout)";
      break;

    case PARAMETER_OUT:
      sout << " (out)";
      break;

    case PARAMETER_RETURN:
      sout << " (return)";
      break;
  }
}

