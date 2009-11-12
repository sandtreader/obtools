//==========================================================================
// ObTools::XMI: operation.cc
//
// UML::Operation functionality
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Constructor
Operation::Operation(XMI::Reader& rdr, XML::Element& xe)
  :BehaviouralFeature(rdr, xe) 
{
  //Read parameter sub-elements from XML source
  read_subelements("UML:Parameter", create_element<Parameter>);

  //Get basic properties
  is_abstract = get_bool_property("isAbstract", 
				  "UML:Operation.isAbstract");

  is_root     = get_bool_property("isRoot", 
				  "UML:Operation.isRoot");

  is_leaf     = get_bool_property("isLeaf", 
				  "UML:Operation.isLeaf");

  string cck  = get_property("concurrency", "UML:Operation.concurrency");

  if (cck=="sequential")
    concurrency=CONCURRENCY_SEQUENTIAL;
  else if (cck=="guarded")
    concurrency=CONCURRENCY_GUARDED;
  else if (cck.empty() || cck=="concurrent")
    concurrency=CONCURRENCY_CONCURRENT;
  else
  {
    reader.warning("Unknown operation concurrency: ", cck);
    concurrency=CONCURRENCY_SEQUENTIAL;  // Safest
  }
}

//--------------------------------------------------------------------------
// Printer
void Operation::print_header(ostream& sout)
{
  BehaviouralFeature::print_header(sout);

  if (is_abstract) sout << " (abstract)";
  if (is_root) sout << " (root)";
  if (is_leaf) sout << " (leaf)";

  switch (concurrency)
  {
    case CONCURRENCY_SEQUENTIAL:
      sout << " (sequential)";
      break;

    case CONCURRENCY_GUARDED:
      sout << " (guarded)";
      break;

    case CONCURRENCY_CONCURRENT:
      // Default - don't clutter
      break;
  }
}


