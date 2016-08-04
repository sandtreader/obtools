//==========================================================================
// ObTools::XMI: element.cc
//
// Generic UML Element functionality
//
// Copyright (c) 2003 Paul Clark.  All rights reserved
// This code comes with NO WARRANTY and is subject to licence agreement
//==========================================================================

#include "ot-uml.h"
#include <stdlib.h>

using namespace ObTools::UML;

//------------------------------------------------------------------------
//Constructor - build from XML element
Element::Element(XMI::Reader& rdr, XML::Element& xe):
  reader(rdr),
  source(xe),
  parent(0)
{
  //Get id
  id = source.get_attr("xmi.id");

  //If id is valid, add myself to the reader's idmap
  if (!id.empty()) reader.record_uml_element(id, this);
}


//--------------------------------------------------------------------------
//Element sub-element reader
//Support for subclass constructor functions - read all subelements
//of given types from XML source, and uses factory func to create them
//Ignores any elements without an 'xmi.id' attribute - these are refs
//Prunes descendant tree at 'prune', if given
void Element::read_subelements(const char *name, ElementFactoryFunc factory,
			       bool id_required, const char *prune)
{
  //Fix 0 prune to empty before cast to string
  if (!prune) prune="";

  //Get all of this tag
  OBTOOLS_XML_FOREACH_PRUNED_DESCENDANT_WITH_TAG(xe, source, name, prune)
    if (!id_required || xe.has_attr("xmi.id"))  // May be reference only
    {
      Element *e = factory(reader, xe);
      subelements.push_back(e);
      e->parent=this;
    }
  OBTOOLS_XML_ENDFOR
}

//------------------------------------------------------------------------
//Function to build things that need valid references (second pass)
//We just pass it down to children
void Element::build_refs()
{
  for(list<Element *>::iterator p=subelements.begin();
      p!=subelements.end();
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
//       <UML:Classifier xmi.idref = 'a6'/>
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
// Gets a cross-referenced element from an idref property
// Allow allows (broken?) Netbeans MDR form with Class/Datatype
// Returns referenced Element, or 0 if not found
Element *Element::get_element_property(const string& attr_name,
				       const string& subelement_name,
				       const string& subsubelement_name)
{
  //Try requested subsubelement
  string idref = get_idref_property(attr_name, subelement_name,
				    subsubelement_name);

  //!XMI: XMI 1.2/UML 1.3 DTD implies only superclass (UML:Classifier,
  //UML:GeneralizableElement) allowed here, but Netbeans MDR (as used
  //in Poseidon) uses UML:Class, UML:DataType etc.  It feels like a
  //misdrafting of the spec (other places do expand superclasses into
  //set-of-all-subclasses for XML), which MDR have unilaterally fixed
  //- but comments welcome!

  //Note we only make allowances for Class, Interface and DataType;
  //Poseidon doesn't issue true Enumerations and Primitives and anything
  //more correct should be using superclasses for everything
  if (idref.empty())
    idref = get_idref_property(attr_name, subelement_name,
			       "UML:Class");
  if (idref.empty())
    idref = get_idref_property(attr_name, subelement_name,
			       "UML:Interface");
  if (idref.empty())
    idref = get_idref_property(attr_name, subelement_name,
			       "UML:DataType");

  if (idref.empty()) return 0;

  Element *e=reader.lookup_uml_element(idref);
  if (e) return e;

  reader.warning("Non-connected type idref in id ", id);
  return 0;
}

//--------------------------------------------------------------------------
// Ditto, but check to be a UML:Classifier
// Looks for UML:Classifier element (and MDR mistakes)
Classifier *Element::get_classifier_property(const string& attr_name,
					     const string& subelement_name)
{
  Element *e = get_element_property(attr_name, subelement_name,
				    "UML:Classifier");
  if (!e) return 0;

  // Make sure it really is some kind of Classifier before returning it
  Classifier *c = dynamic_cast<Classifier *>(e);
  if (c) return c;

  reader.warning("Bogus classifier idref found in id ", id);
  return 0;
}

//--------------------------------------------------------------------------
// Ditto, but check to be a UML:GeneralizableElement
// Looks for UML:GeneralizableElement element (and MDR mistakes)
GeneralizableElement *Element::get_ge_property(const string& attr_name,
					       const string& subelement_name)
{
  Element *e = get_element_property(attr_name, subelement_name,
				    "UML:GeneralizableElement");
  if (!e) return 0;

  // Make sure it really is some kind of GE before returning it
  GeneralizableElement *ge = dynamic_cast<GeneralizableElement *>(e);
  if (ge) return ge;

  reader.warning("Bogus GE idref found in id ", id);
  return 0;
}

//--------------------------------------------------------------------------
// Element header printer
void Element::print_header(ostream& sout)
{
  sout << source.name;
}

//--------------------------------------------------------------------------
// Element printer - indents to indent, calls down to print_header, then
// prints sub-elements at indent+2
void Element::print(ostream& sout, int indent)
{
  //Indent first line and downcall to child header printer
  sout << string(indent, ' ');
  print_header(sout);
  sout << endl;

  //Print elements through virtual printer
  for(list<Element *>::iterator p=subelements.begin();
      p!=subelements.end();
      p++)
    (*p)->print(sout, indent+2);
}

//--------------------------------------------------------------------------
// Element destructor
Element::~Element()
{
  //Delete subelements through virtual destructor
  for(list<Element *>::iterator p=subelements.begin();
      p!=subelements.end();
      p++)
    delete *p;
}




