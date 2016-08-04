//==========================================================================
// ObTools::XMI: enumeration.cc
//
// UML::Enumeration functionality
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Constructor
Enumeration::Enumeration(XMI::Reader& rdr, XML::Element& xe)
  :DataType(rdr, xe)
{
  // Read all EnumerationLiterals (ignoring UML:Enumeration.literal cruft)
  OBTOOLS_XML_FOREACH_DESCENDANT_WITH_TAG(lite, xe,
                                                 "UML:EnumerationLiteral")
    string litname = lite.get_attr("name");
    if (!litname.empty()) literals.push_back(litname);
  OBTOOLS_XML_ENDFOR
}

//--------------------------------------------------------------------------
// Printer
void Enumeration::print_header(ostream& sout)
{
  GeneralizableElement::print_header(sout);

  sout << " [ ";

  for(list<string>::iterator p = literals.begin();
      p!=literals.end();
      p++)
    sout << "'" << *p << "' ";

  sout << "]";
}



