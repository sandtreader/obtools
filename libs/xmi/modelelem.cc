//==========================================================================
// ObTools::XMI: modelelem.cc
//
// UML ModelElement functionality
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//------------------------------------------------------------------------
//Constructor - build from XML element
ModelElement::ModelElement(XMI::Reader& rdr, XML::Element& xe):
  Element(rdr, xe)
{
  //Get  name
  name = get_property("name", "UML:ModelElement.name");

  //Get visibility
  string vis = get_property("visibility", "UML:ModelElement.visibility");

  if (vis=="public")
    visibility=VISIBILITY_PUBLIC;
  else if (vis=="protected")
    visibility=VISIBILITY_PROTECTED;
  else if (vis=="private")
    visibility=VISIBILITY_PRIVATE;
  else if (vis.empty() || vis=="package")
    visibility=VISIBILITY_PACKAGE;
  else
  {
    reader.warning("Unknown element visibility: ", vis);
    visibility=VISIBILITY_PRIVATE;  //safest
  }
}    

//--------------------------------------------------------------------------
// Second-pass reference fix
void ModelElement::build_refs()
{
  Element::build_refs();

  //Get stereotype ref
  //!!!
}

//--------------------------------------------------------------------------
// ModelElement header printer - adds name
void ModelElement::print_header(ostream& sout)
{
  Element::print_header(sout);

  switch (visibility)
  {
    case VISIBILITY_PUBLIC:
      sout << " public";
      break;

    case VISIBILITY_PROTECTED:
      sout << " protected";
      break;
    
    case VISIBILITY_PRIVATE:
      sout << " private";
      break;

    case VISIBILITY_PACKAGE:
      //default - don't clutter it
      break;
  }

  sout << " '" << name << "'";

  //Print stereotype!!!
}






