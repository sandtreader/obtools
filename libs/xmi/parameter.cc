//==========================================================================
// ObTools::XMI: parameter.cc
//
// UML::Parameter functionality
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
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
}

//--------------------------------------------------------------------------
// Second-pass reference fix
void Parameter::build_refs()
{
  ModelElement::build_refs();

  //!!! Get type ref

}

//--------------------------------------------------------------------------
// Printer
void Parameter::print_header(ostream& sout)
{
  ModelElement::print_header(sout);

  //!!! Print type

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
