//==========================================================================
// ObTools::XMI: association.cc
//
// UML::Association functionality
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Constructor
Association::Association(XMI::Reader& rdr, XML::Element& xe)
  :GeneralizableElement(rdr, xe) 
{
  //Read AssociationEnd sub-elements from XML source
  read_subelements("UML:AssociationEnd", create_element<AssociationEnd>);

  //Bang them into the connections vector to make life easy 
  //Also fix up the connection_index in each one
  int i=0;
  for(list<Element *>::iterator p=subelements.begin();
      p!=subelements.end();
      p++, i++)
  {
    AssociationEnd *ae = dynamic_cast<AssociationEnd *>(*p);
    if (ae)
    {
      ae->connection_index = i;
      connections.push_back(ae);
    }
  }
}


