//==========================================================================
// ObTools::XMI: reader.cc
//
// XMI reader class
//
// Copyright (c) 2003 Object Toolsmiths Limited.  All rights reserved
//==========================================================================

#include "ot-xmi.h"
using namespace ObTools::XMI;
using namespace ObTools; 

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
// Lookup a class in the classmap by ID
// Returns 0 if failed
UML::Class *Reader::lookup_class(const string& id)
{
  map<string,UML::Class *>::iterator p=classmap.find(id);
  if (p!=classmap.end())
    return p->second;
  else
    return 0;
}

//------------------------------------------------------------------------
// Read an attribute from given type element containing 
// UML:Class or UML:Datatype reference
// Returns 0 if failed
UML::Class *Reader::get_type(XML::Element& te)
{
  //Now look for either UML::Class or UML::DataType for the value
  string tid;
  XML::Element &ce = te.get_child("UML:Class");
  if (ce.valid())
    tid = ce.get_attr("xmi.idref");
  else
  {
    XML::Element &dte = te.get_child("UML:DataType");
    if (dte.valid())
      tid = dte.get_attr("xmi.idref");
    else
    {
      warning("Type definition with no UML:Class or UML:DataType", "");
      return 0;
    }
  }

  //Check tid
  if (tid.empty())
  {
    warning("UML:Class or UML:Datatype reference with no idref", "");
    return 0;
  }  

  //Lookup tid in classmap
  UML::Class *c = lookup_class(tid);
  if (!c)
  {
    warning("Unknown type reference ", tid);
    return 0;
  }

  return c;
}

//------------------------------------------------------------------------
// Read an attribute from given <UML:Attribute> element
// Returns 0 if failed
UML::Attribute *Reader::read_attribute(XML::Element& ae)
{
  string id = ae.get_attr("xmi.id");  // Only used for errors

  //It must have a name
  string name = ae.get_attr("name");
  if (name.empty())
  {
    warning("Attribute with no name ignored - id ", id);
    return 0;
  }

  //Find 'UML:StructuralFeature.type' part
  XML::Element &sfe = ae.get_child("UML:StructuralFeature.type");
  if (!sfe.valid())
  {
    warning("Attribute with no type definition ignored - id ", id);
    return 0;
  }

  UML::Class *c = get_type(sfe);
  if (!c)
  {
    warning("Attribute with bad type definition ignored - id ", id);
    return 0;
  }

  //Now we have enough to create it
  UML::Attribute *a = new UML::Attribute(name, c);

  return a;
}

//------------------------------------------------------------------------
// Capture the class from given <UML:Class> or <UML:DataType> element
// First pass - just grabs name and enters id into the classmap
// read_class does the real work later
void Reader::scan_class(XML::Element& ce, UML::ClassKind kind)
{
  //Class definitions must have an ID - otherwise it's a reference
  //Silently ignore references
  string id = ce.get_attr("xmi.id");
  if (id.empty()) return;
  
  //It must have a name
  string name = ce.get_attr("name");
  if (name.empty())
  {
    warning("Class/DataType with no name ignored - id ", id);
    return;
  }

  //That's enough to commit to
  UML::Class *c = new UML::Class(id, name, kind);

  // Enter in the classmap
  classmap[id]=c;
}

//------------------------------------------------------------------------
// Read a class from given <UML:Class> element
// Returns 0 if failed
// Second pass - fills in details now classmap is full
UML::Class *Reader::read_class(XML::Element& ce)
{
  //Class definitions must have an ID - otherwise it's a reference
  //Silently ignore references
  string id = ce.get_attr("xmi.id");
  if (id.empty()) return 0;
  
  //Lookup class by id
  UML::Class *c = lookup_class(id);
  if (!c)
  {
    warning("Classmap entry disappeared for id ", id);
    return 0;
  }

  if (ce.get_attr_bool("isAbstract"))
    c->kind = UML::CLASS_ABSTRACT;

  if (ce.has_attr("stereotype"))
    c->stereotype = ce.get_attr("stereotype");

  //Look for attributes
  OBTOOLS_XML_FOREACH_DESCENDANT_WITH_TAG(ae, ce, "UML:Attribute")
    UML::Attribute *a = read_attribute(ae);
    if (a) c->attributes.push_back(a);
  OBTOOLS_XML_ENDFOR

  return c;
}

//------------------------------------------------------------------------
// Read an association from given <UML:Association> element
// Returns 0 if failed
UML::Association *Reader::read_association(XML::Element& ae)
{
  return 0;
}

//------------------------------------------------------------------------
// Read a package from given <UML:Model> or <UML:Package> element
// Returns 0 if failed
UML::Package *Reader::read_package(XML::Element& pe)
{
  if (!pe.has_attr("name"))
  {
    warning("Package with missing name ignored - id ", pe.get_attr("xmi.id"));
    return 0;
  }

  UML::Package *package = new UML::Package(pe.get_attr("name"));

  //Get all classes at this package level (but ignoring XMI cruft
  //inbetween) 
  OBTOOLS_XML_FOREACH_PRUNED_DESCENDANT_WITH_TAG(ce, pe, "UML:Class", 
						 "UML:Package")
    UML::Class *c = read_class(ce);
    if (c) package->classes.push_back(c);
  OBTOOLS_XML_ENDFOR

  //Likewise datatypes
  OBTOOLS_XML_FOREACH_PRUNED_DESCENDANT_WITH_TAG(ce, pe, "UML:DataType", 
						 "UML:Package")
    UML::Class *c = read_class(ce);
    if (c) package->classes.push_back(c);
  OBTOOLS_XML_ENDFOR

  //Get all associations, likewise
  OBTOOLS_XML_FOREACH_PRUNED_DESCENDANT_WITH_TAG(ae, pe, "UML:Association",
						 "UML:Package")
    UML::Association *a = read_association(ae);
    if (a) package->associations.push_back(a);
  OBTOOLS_XML_ENDFOR

  //Get sub-packages, first level only (self-pruned)
  OBTOOLS_XML_FOREACH_PRUNED_DESCENDANT_WITH_TAG(spe, pe, "UML:Package",
						 "UML:Package")
    UML::Package *p = read_package(spe);
    if (p) package->packages.push_back(p);
  OBTOOLS_XML_ENDFOR

  return package;
}

//------------------------------------------------------------------------
// Parse from given input stream
void Reader::read_from(istream& s) throw (ParseFailed)
{
  XML::Parser parser;

  // Add UML namespaces (seen both of these!)
  parser.fix_namespace("org.omg.xmi.namespace.UML", "UML");
  parser.fix_namespace("org.omg/UML1.3", "UML");

  try
  {
    s >> parser;
  }
  catch (XML::ParseFailed)
  {
    error("XML parsing failed");
  }

  XML::Element& root=parser.get_root();

  //Make sure it's XMI
  if (root.name != "XMI") 
    error("Not an <XMI> file - root element is ", root.name);

  //Get XMI.content
  XML::Element &xmi_content = root.get_child("XMI.content");
  if (!xmi_content.valid()) error("No <XMI.content> in <XMI>");

  //Get UML model - assume only one
  XML::Element &modele = xmi_content.get_child("UML:Model");
  if (!modele.valid()) error("No <UML:Model> in <XMI.content>");

  //Prescan for class ids throughout the model, and build class map
  OBTOOLS_XML_FOREACH_DESCENDANT_WITH_TAG(ce, modele, "UML:Class")
    scan_class(ce, UML::CLASS_CONCRETE);
  OBTOOLS_XML_ENDFOR

  //Ditto for datatypes 
  OBTOOLS_XML_FOREACH_DESCENDANT_WITH_TAG(ste, modele, "UML:DataType")
    scan_class(ste, UML::CLASS_DATATYPE);
  OBTOOLS_XML_ENDFOR

  //Now read model properly
  model = read_package(modele);
}

//------------------------------------------------------------------------
// >> operator to read from istream
istream& ObTools::XMI::operator>>(istream& s, Reader& p) throw (ParseFailed)
{ 
  p.read_from(s);
}



