//==========================================================================
// ObTools::XMI: assocend.cc
//
// UML::AssociationEnd functionality
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Constructor
AssociationEnd::AssociationEnd(XMI::Reader& rdr, XML::Element& xe)
  :ModelElement(rdr, xe) 
{
  //Get basic properties
  is_ordered = (get_property("ordering", "UML:AssociationEnd.ordering")
		== "ordered");

  is_navigable = get_bool_property("isNavigable", 
				  "UML:AssociationEnd.isNavigable");

  multiplicity = get_multiplicity();

  string ak  = get_property("aggregation", "UML:AssociationEnd.aggregation");

  if (ak.empty() || ak=="none")
    aggregation=AGGREGATION_NONE;
  else if (ak=="aggregate")
    aggregation=AGGREGATION_AGGREGATE;
  else if (ak.empty() || ak=="composite")
    aggregation=AGGREGATION_COMPOSITE;
  else
  {
    reader.warning("Unknown association-end aggregation: ", ak);
    aggregation=AGGREGATION_NONE;  // Safest
  }
}

//--------------------------------------------------------------------------
// Second-pass reference fix
void AssociationEnd::build_refs()
{
  ModelElement::build_refs();

  //Find participant!!!
}


//--------------------------------------------------------------------------
// Printer
void AssociationEnd::print_header(ostream& sout)
{
  ModelElement::print_header(sout);

  if (is_ordered) sout << " (ordered)";
  //It's actually more interesting if it's _not_ navigable!
  if (!is_navigable) sout << " (non-navigable)";

  switch (aggregation)
  {
    case AGGREGATION_NONE:
      // Default - don't clutter
      break;

    case AGGREGATION_AGGREGATE:
      sout << " (aggregate)";
      break;

    case AGGREGATION_COMPOSITE:
      sout << " (composite)";
      break;
  }

  //!!!Print participant

  sout << " " << multiplicity;
}


