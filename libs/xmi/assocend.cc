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
AssociationEnd::AssociationEnd(XMI::Reader& rdr, XML::Element& xe):
  ModelElement(rdr, xe), participant(0), connection_index(0)
{
  //Get basic properties
  is_ordered = (get_property("ordering", "UML:AssociationEnd.ordering")
		== "ordered");

  is_navigable = get_bool_property("isNavigable", 
				  "UML:AssociationEnd.isNavigable");

  multiplicity = Multiplicity::read_from(source);

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

  participant = get_classifier_property("participant", 
					"UML:AssociationEnd.participant");
  if (!participant)
    reader.error("Can't get participant in AssociationEnd id ", id);

  //Fix up Classifier's associations list to point back to me
  participant->association_ends.push_back(this);
}

//--------------------------------------------------------------------------
// Printer
void AssociationEnd::print_header(ostream& sout)
{
  ModelElement::print_header(sout);

  if (participant)
    sout << " -> " << participant->name;

  sout << multiplicity;

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
}

//--------------------------------------------------------------------------
//Get the 'other' end of the association (only works for 2 ends)
AssociationEnd *AssociationEnd::get_other_end()
{
  Association *pa = dynamic_cast<Association *>(parent);
  if (!pa) return 0;

  if (pa->connections.size() != 2) return 0;

  return pa->connections[1-connection_index];
}
