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
  name = source.get_attr("name");

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
// or 'xmi.value' attribute of given sub-element of source
//
//   <UML:Class isAbstract='true' ...>
// or
//   <UML:Class ...>
//     ...
//     <UML:GeneralizableElement.isAbstract xmi.value="true"/>
//     ...
//
//
// Attributes take priority
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
    return sube.get_attr("xmi.value");
  else
    return "";
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
// Gets a type element from the type properties
// Fills in Type structure, returns whether successful
bool Element::get_type(Type& t)
{
  //!XMI: XMI 1.2/UML 1.3 DTD implies UML:Classifier allowed here, not
  //UML:Class and UML:DataType - but a Poseidon model is as below...

  //Try class ref
  bool is_class = true;
  string idref = get_idref_property("type",
				    "UML:StructuralFeature.type",
				    "UML:Class");
  if (idref.empty())
  {
    is_class = false;

    //Try datatype ref
    idref = get_idref_property("type",
			       "UML:StructuralFeature.type",
			       "UML:DataType");
  }

  if (idref.empty())
  {
    reader.warning("Type idref not found in ", name);
    return false;
  }

  Element *e=reader.lookup_element(idref);
  if (!e)
  {
    reader.warning("Bad idref in ", name);
    return false;
  }

  t.is_class = is_class;

  if (is_class)
  {
    t.c = dynamic_cast<Class *>(e);  
    if (!t.c) reader.error("Bogus idref in Class ", name);
  }
  else
  {
    t.dt = dynamic_cast<DataType *>(e);
    if (!t.dt) reader.error("Bogus idref in DataType ", name);
  }

  //Read multiplicity!

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




