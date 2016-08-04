//==========================================================================
// ObTools::XMI: types.cc
//
// Basic UML datatypes support
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-uml.h"
#include <stdlib.h>

using namespace ObTools::UML;

//==========================================================================
// UML::Multiplicity struct

//--------------------------------------------------------------------------
// Reads multiplicity from UML:Multiplicity subelement
// Returns default (1,1) if not found
Multiplicity Multiplicity::read_from(XML::Element& pare,
				     XMI::Reader& reader)
{
  Multiplicity m;
  XML::Element *real_me;
  XML::Element& me = pare.get_descendant("UML:Multiplicity");

  // Check for ID ref
  string idref = me["xmi.idref"];
  if (!idref.empty())
  {
    real_me = reader.lookup_xml_element(idref);
    if (!real_me) return m;
  }
  else real_me = &me;  // Use this one

  if (real_me->valid())
  {
    XML::Element& mr = real_me->get_descendant("UML:MultiplicityRange");
    if (mr.valid())
    {
      //Look either in attributes or sub-elements
      if (mr.has_attr("lower"))
	m.lower = mr.get_attr_int("lower");
      else
      {
	XML::Element &mrl = mr.get_child("UML:MultiplicityRange.lower");
	if (mrl.valid() && !mrl.content.empty())
	  m.lower = atoi(mrl.content.c_str());
      }

      if (mr.has_attr("upper"))
	m.upper = mr.get_attr_int("upper");
      else
      {
	XML::Element &mru = mr.get_child("UML:MultiplicityRange.upper");
	if (mru.valid() && !mru.content.empty())
	  m.upper = atoi(mru.content.c_str());
      }
    }
  }

  return m;
}

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

//==========================================================================
// UML::Expression struct

//Method to read from XMI - pass parent element of UML:Expression
Expression Expression::read_from(XML::Element &pare)
{
  Expression exp;

  // Look for UML:expression subelement
  // !XMI: Don't yet handle all the myriad subtypes
  XML::Element& expe = pare.get_child("UML:Expression");
  if (expe.valid())
  {
    // Read from either 'language' attribute or equivalent subelement
    if (expe.has_attr("language"))
      exp.language = expe.get_attr("language");
    else
    {
      XML::Element& lange = expe.get_child("UML:Expression.language");
      if (lange.valid())
	exp.language = lange.content;
    }

    // Ditto for 'body'
    if (expe.has_attr("body"))
      exp.body = expe.get_attr("body");
    else
    {
      XML::Element& bodye = expe.get_child("UML:Expression.body");
      if (bodye.valid())
	exp.body = bodye.content;
    }
  }

  return exp;
}




