//==========================================================================
// ObTools::XMI: reader.cc
//
// XMI reader class
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmi.h"
using namespace ObTools::XMI;
using namespace ObTools::UML;

//--------------------------------------------------------------------------
// Destructor
Reader::~Reader()
{
  if (model) delete model;
}

//------------------------------------------------------------------------
// Warning handler
void Reader::warning(const char *warn, const string& detail) 
{
  serr << warn << detail << endl;
}

//------------------------------------------------------------------------
// Fatal error handler
void Reader::error(const char *err, const string& detail) 
     throw (ParseFailed)
{
  serr << err << detail << endl;
  throw ParseFailed();
}

//------------------------------------------------------------------------
// Read an attribute from given <UML:Attribute> element
// Returns 0 if failed
Attribute *Reader::read_attribute(ObTools::XML::Element& ae)
{
  //It must have a name
  string name = ae.get_attr("name");
  if (name.empty())
  {
    warning("Attribute with no name ignored - id ", ae.get_attr("xmi.id"));
    return 0;
  }

  //  Attribute *a = new Attribute(id, !!!class from map);

  return 0;
}

//------------------------------------------------------------------------
// Read a class from given <UML:Class> element
// Returns 0 if failed
Class *Reader::read_class(ObTools::XML::Element& ce)
{
  //Class definitions must have an ID - otherwise it's a reference
  //Silently ignore references
  string id = ce.get_attr("xmi.id");
  if (id.empty()) return 0;
  
  //It must have a name
  string name = ce.get_attr("name");
  if (name.empty())
  {
    warning("Class with no name ignored - id ", id);
    return 0;
  }

  //That's enough to commit to
  Class *c = new Class(id, name);

  if (ce.get_attr_bool("isAbstract"))
    c->kind = CLASS_ABSTRACT;

  if (ce.has_attr("stereotype"))
    c->stereotype = ce.get_attr("stereotype");

  //Look for attributes
  OBTOOLS_XML_FOREACH_DESCENDANT_WITH_TAG(ae, ce, "UML:Attribute")
    Attribute *a = read_attribute(ae);
    if (a) c->attributes.push_back(a);
  OBTOOLS_XML_ENDFOR

  return c;
}

//------------------------------------------------------------------------
// Read an association from given <UML:Association> element
// Returns 0 if failed
Association *Reader::read_association(ObTools::XML::Element& ae)
{
  return 0;
}

//------------------------------------------------------------------------
// Read a package from given <UML:Model> or <UML:Package> element
// Returns 0 if failed
Package *Reader::read_package(ObTools::XML::Element& pe)
{
  if (!pe.has_attr("name"))
  {
    warning("Package with missing name ignored - id ", pe.get_attr("xmi.id"));
    return 0;
  }

  Package *package = new Package(pe.get_attr("name"));

  //Get all classes at this package level (but ignoring XMI cruft
  //inbetween)
  OBTOOLS_XML_FOREACH_PRUNED_DESCENDANT_WITH_TAG(ce, pe, "UML:Class", 
						 "UML:Package")
    Class *c = read_class(ce);
    if (c) package->classes.push_back(c);
  OBTOOLS_XML_ENDFOR

  //Get all associations, likewise
  OBTOOLS_XML_FOREACH_PRUNED_DESCENDANT_WITH_TAG(ae, pe, "UML:Association",
						 "UML:Package")
    Association *a = read_association(ae);
    if (a) package->associations.push_back(a);
  OBTOOLS_XML_ENDFOR

  //Get sub-packages, first level only (self-pruned)
  OBTOOLS_XML_FOREACH_PRUNED_DESCENDANT_WITH_TAG(spe, pe, "UML:Package",
						 "UML:Package")
    Package *p = read_package(spe);
    if (p) package->packages.push_back(p);
  OBTOOLS_XML_ENDFOR

  return package;
}

//------------------------------------------------------------------------
// Parse from given input stream
void Reader::read_from(istream& s) throw (ParseFailed)
{
  ObTools::XML::Parser parser;

  // Add UML namespaces (seen both of these!)
  parser.fix_namespace("org.omg.xmi.namespace.UML", "UML");
  parser.fix_namespace("org.omg/UML1.3", "UML");

  try
  {
    s >> parser;
  }
  catch (ObTools::XML::ParseFailed)
  {
    error("XML parsing failed");
  }

  ObTools::XML::Element& root=parser.get_root();

  //Make sure it's XMI
  if (root.name != "XMI") error("Not an <XMI> file - root element is ", root.name);

  //Get XMI.content
  ObTools::XML::Element &xmi_content = root.get_child("XMI.content");
  if (!xmi_content.valid()) error("No <XMI.content> in <XMI>");

  //Get UML model - assume only one
  ObTools::XML::Element &uml_model = xmi_content.get_child("UML:Model");
  if (!uml_model.valid()) error("No <UML:Model> in <XMI.content>");
  
  model = read_package(uml_model);
}

//------------------------------------------------------------------------
// >> operator to read from istream
istream& ObTools::XMI::operator>>(istream& s, Reader& p) throw (ParseFailed)
{ 
  p.read_from(s);
}



