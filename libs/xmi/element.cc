//==========================================================================
// ObTools::XMI: element.cc
//
// Generic UML Element functionality
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-uml.h"
using namespace ObTools::UML;

//------------------------------------------------------------------------
//Constructor - build from XML element
Element::Element(XMI::Reader& rdr, XML::Element& xe):
  reader(rdr),
  source(xe)
{
  //Get id and name
  id = source.get_attr("xmi.id");
  name = get_property("name", "UML:ModelElement.name");

  //Get visibility
  string vis = get_property("visibility", "UML:ModelElement.visibility");

  if (vis.empty() || vis=="public")
    visibility=UML::ELEMENT_PUBLIC;
  else if (vis=="protected")
    visibility=UML::ELEMENT_PROTECTED;
  else if (vis=="private")
    visibility=UML::ELEMENT_PRIVATE;
  else
  {
    reader.warning("Unknown element visibility: ", vis);
    visibility=UML::ELEMENT_PUBLIC;
  }

  //If id is valid, add myself to the reader's idmap
  if (!id.empty()) reader.record_element(id, this);
}    

//------------------------------------------------------------------------
//Function to build things that need valid references (second pass)
//We just pass it down to children
void Element::build_refs()
{
  for(list<Element *>::iterator p=elements.begin();
      p!=elements.end();
      p++)
    (*p)->build_refs();
}

//------------------------------------------------------------------------
// Gets UML element 'property' either from given attribute of source
// or 'xmi.value' attribute, or content, of given sub-element of source
//
//   <UML:Class isAbstract='true' ...>
// or
//   <UML:Class ...>
//     <UML:GeneralizableElement.isAbstract xmi.value="true"/>
// or
//   <UML:Class ...>
//     <UML:ModelElement.name>foo</UML:ModelElement.name>
//
// Attributes take priority, then subelement xmi.value, then subelement
// content
//
// Returns "" if not found
//
// [Note to future DTD designers - please choose attributes OR subelements, 
//  not both!]
string Element::get_property(const string& attr_name,
			     const string& subelement_name)
{
  //Try attribute
  string v = source.get_attr(attr_name);
  if (!v.empty()) return v;

  //Try subelement
  XML::Element& sube = source.get_child(subelement_name);
  if (sube.valid())
  {
    v = sube.get_attr("xmi.value");
    if (v.empty()) v=sube.content;
    return v;
  }
  else return "";
}

//------------------------------------------------------------------------
// Gets UML element boolean 'property' as above
// Anything other than 'true' is considered 'false'
bool Element::get_bool_property(const string& attr_name,
				const string& subelement_name)
{
  return get_property(attr_name, subelement_name) == "true";
}

//------------------------------------------------------------------------
// Gets UML element integer 'property' as above
// Returns default if not available, 0 if not a number
bool Element::get_int_property(const string& attr_name,
			       const string& subelement_name,
			       int def)
{
  string v = get_property(attr_name, subelement_name);
  if (v.empty()) return def;
  return atoi(v.c_str());
}

//------------------------------------------------------------------------
// Gets a reference 'property', either from given attribute of source
// or 'xmi.idref' attribute of given sub-sub-element of given sub-element of
// source (phew!)
// e.g.
//
//   <UML:Class stereotype='a6' ...>
// or
//   <UML:Class ...>
//     ...
//     <UML:ModelElement.stereotype>
//       <UML:Stereotype xmi.idref = 'a6'/>
//     </UML:ModelElement.stereotype>
//     ...
//
// Attributes take priority
// sub-sub-element is optional, to cope with single-layer form 
// Returns "" if not found
string Element::get_idref_property(const string& attr_name,
				   const string& subelement_name,
				   const string& subsubelement_name)
{
  //Try attribute
  string v = source.get_attr(attr_name);
  if (!v.empty()) return v;

  //Read through subelement
  XML::Element& sube = source.get_child(subelement_name);
  if (sube.valid())
  {
    //If subsubelement not given, go for this one
    if (subsubelement_name.empty())
      return sube.get_attr("xmi.idref");
    else
    {
      //Try to open subsubelement
      XML::Element& subsube = sube.get_child(subsubelement_name);
      if (subsube.valid())
	return subsube.get_attr("xmi.idref");
    }
  }

  return "";
}

//--------------------------------------------------------------------------
// Reads multiplicity from properties
// Returns default (1,1) if not found
Multiplicity Element::get_multiplicity()
{
  Multiplicity m;
  XML::Element& me = source.get_descendant("UML:Multiplicity");
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
// Gets a type element from the type properties
// Fills in Type structure, returns whether successful
bool Element::get_type(Type& t)
{
  //!XMI: XMI 1.2/UML 1.3 DTD implies UML:Classifier allowed here, not
  //UML:Class and UML:DataType - but a Poseidon model is as below...
  //We allow any of them and work out the type later

  //Try 'type' attribute, or UML:Class or UML:Datatype subelements
  string idref = get_idref_property("type",
				    "UML:StructuralFeature.type",
				    "UML:Class");
  if (idref.empty())
    idref = get_idref_property("type",
			       "UML:StructuralFeature.type",
			       "UML:DataType");

  if (idref.empty())
    idref = get_idref_property("type",
			       "UML:StructuralFeature.type",
			       "UML:Classifier");

  if (idref.empty())
  {
    reader.warning("Type idref not found in ", name);
    return false;
  }

  Element *e=reader.lookup_element(idref);
  if (!e)
  {
    reader.warning("Non-connected type idref in ", name);
    return false;
  }

  // Find out what we caught - check actual types here rather than
  // relying on reference name - 'type' is ambiguous anyway
  t.c = dynamic_cast<Class *>(e);  
  if (t.c)
    t.is_class = true;
  else
  {
    t.dt = dynamic_cast<DataType *>(e);
    if (t.dt)
      t.is_class = false;
    else
    {
      reader.warning("Bogus type idref in ", name);
      return false;
    }
  }

  //Read multiplicity
  t.multi = get_multiplicity();

  return true;
}

//--------------------------------------------------------------------------
// Element sub-element printer
void Element::print_subelements(ostream& sout, int indent)
{
  //Print elements through virtual printer
  for(list<Element *>::iterator p=elements.begin();
      p!=elements.end();
      p++)
    (*p)->print(sout, indent);
}

//--------------------------------------------------------------------------
// Element printer - defaults to XML name, id, name and sub-elements
void Element::print(ostream& sout, int indent)
{
  sout << string(indent, ' ');
  sout << source.name << " '" << name << "'";

  if (!id.empty())
    sout << " (" << id << ")";

  sout << endl;

  //List subelements (if any)
  print_subelements(sout, indent+2);
}

//--------------------------------------------------------------------------
// Element destructor
Element::~Element()
{
  //Delete subelements through virtual destructor
  for(list<Element *>::iterator p=elements.begin();
      p!=elements.end();
      p++)
    delete *p;
}




