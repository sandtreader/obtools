//==========================================================================
// ObTools::XMI: package.cc
//
// UML::Package functionality
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Constructor
Package::Package(XMI::Reader& rdr, XML::Element& xe)
  :Element(rdr, xe) //Element does the basics
{
  //Get all classes at this package level (but ignoring XMI cruft
  //inbetween) 
  OBTOOLS_XML_FOREACH_PRUNED_DESCENDANT_WITH_TAG(ce, xe, "UML:Class", 
						 "UML:Package")
    if (ce.has_attr("xmi.id"))  // May be reference only
    {
      Class *c = new Class(rdr, ce);
      elements.push_back(c);
      classes.push_back(c);
    }
  OBTOOLS_XML_ENDFOR

  //Likewise datatypes 
  OBTOOLS_XML_FOREACH_PRUNED_DESCENDANT_WITH_TAG(de, xe, "UML:DataType", 
						 "UML:Package")
    if (de.has_attr("xmi.id"))  // May be reference only
    {
      DataType *t = new DataType(rdr, de);
      elements.push_back(t);
    }
  OBTOOLS_XML_ENDFOR

  //Likewise stereotypes 
  OBTOOLS_XML_FOREACH_PRUNED_DESCENDANT_WITH_TAG(se, xe, "UML:Stereotype", 
						 "UML:Package")
    if (se.has_attr("xmi.id"))  // May be reference only
    {
      Stereotype *s = new Stereotype(rdr, se);
      elements.push_back(s);
    }
  OBTOOLS_XML_ENDFOR

  //Get all associations - assume all wanted
  OBTOOLS_XML_FOREACH_PRUNED_DESCENDANT_WITH_TAG(ae, xe, "UML:Association",
						 "UML:Package")
    Association *a = new Association(rdr, ae);
    elements.push_back(a);
    associations.push_back(a);
  OBTOOLS_XML_ENDFOR

  //Get sub-packages, first level only (self-pruned)
  OBTOOLS_XML_FOREACH_PRUNED_DESCENDANT_WITH_TAG(pe, xe, "UML:Package",
						 "UML:Package")
    Package *p = new Package(rdr, pe);
    elements.push_back(p);
    packages.push_back(p);
  OBTOOLS_XML_ENDFOR
}

//--------------------------------------------------------------------------
// Printer
void Package::print(ostream& sout, int indent=0)
{
  Element::print(sout, indent);
}

