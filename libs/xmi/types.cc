//==========================================================================
// ObTools::XMI: types.cc
//
// Basic UML datatypes support
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//==========================================================================
// UML::Multiplicity struct

//--------------------------------------------------------------------------
// Multiplicity streamer
ostream& ObTools::UML::operator<<(ostream& s, const Multiplicity& m)
{
  //If 1..1, keep stumm
  if (m.upper==1 && m.lower==1) return s;

  s << '[' << m.lower << "..";

  if (m.upper<0)
    s << '*';
  else
    s << m.upper;

  s << ']';

  return s;
}
