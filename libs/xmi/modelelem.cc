//==========================================================================
// ObTools::XMI: modelelem.cc
//
// UML ModelElement functionality
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//------------------------------------------------------------------------
//Constructor - build from XML element
ModelElement::ModelElement(XMI::Reader& rdr, XML::Element& xe):
  Element(rdr, xe), stereotype(0)
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
  string idref = get_idref_property("stereotype",
				    "UML:ModelElement.stereotype",
				    "UML:Stereotype");
  if (!idref.empty())
  {
    Element *e=reader.lookup_uml_element(idref);
    if (e)
    {
      stereotype = dynamic_cast<Stereotype *>(e);
      if (!stereotype)
	reader.warning("Bogus stereotype idref in id ", id);
    }
    else
      reader.warning("Non-connected stereotype idref in id ", id);
  }
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

  if (stereotype)
    sout << " <<" << stereotype->name << ">>";
}






