//==========================================================================
// ObTools::XMI: types.cc
//
// Basic UML datatypes support
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//==========================================================================
// UML::Multiplicity struct

//--------------------------------------------------------------------------
// Reads multiplicity from UML:Multiplicity subelement
// Returns default (1,1) if not found
Multiplicity Multiplicity::read_from(XML::Element& pare)
{
  Multiplicity m;
  XML::Element& me = pare.get_descendant("UML:Multiplicity");
  if (me.valid())
  {
    XML::Element& mr = me.get_descendant("UML:MultiplicityRange");
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




