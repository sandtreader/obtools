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
// Constructor
Reader::Reader(ostream&s):
  serr(s),
  xml_parser(s),
  model(0)
{
  // Add UML namespaces to parser (seen both of these!)
  xml_parser.fix_namespace("org.omg.xmi.namespace.UML", "UML");
  xml_parser.fix_namespace("org.omg/UML1.3", "UML");
}

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
//Record an ID to element mappimg
void Reader::record_element(const string& id, UML::Element *e)
{
  idmap[id]=e;
}

//------------------------------------------------------------------------
// Lookup an element in the idmap by ID
// Returns 0 if failed
UML::Element *Reader::lookup_element(const string& id)
{
  map<string,UML::Element *>::iterator p=idmap.find(id);
  if (p!=idmap.end())
    return p->second;

  warning("Bad element reference idref ", id);
  return 0;
}

#if 0
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

#endif

//------------------------------------------------------------------------
// Parse from given input stream
void Reader::read_from(istream& s) throw (ParseFailed)
{
  try
  {
    s >> xml_parser;
  }
  catch (XML::ParseFailed)
  {
    error("XML parsing failed");
  }

  XML::Element& root=xml_parser.get_root();

  //Make sure it's XMI
  if (root.name != "XMI") 
    error("Not an <XMI> file - root element is ", root.name);

  //Get XMI.content
  XML::Element &xmi_content = root.get_child("XMI.content");
  if (!xmi_content.valid()) error("No <XMI.content> in <XMI>");

  //Get UML model - assume only one
  XML::Element &modele = xmi_content.get_child("UML:Model");
  if (!modele.valid()) error("No <UML:Model> in <XMI.content>");

  //Now read model into a UML Package
  model = new UML::Package(*this, modele);
}

//------------------------------------------------------------------------
// >> operator to read from istream
istream& ObTools::XMI::operator>>(istream& s, Reader& p) throw (ParseFailed)
{ 
  p.read_from(s);
}



