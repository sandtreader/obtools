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

  //Now all ids loaded, fix up things that require references
  // - second pass to avoid forward reference problems
  model->build_refs();
}

//------------------------------------------------------------------------
// >> operator to read from istream
istream& ObTools::XMI::operator>>(istream& s, Reader& p) throw (ParseFailed)
{ 
  p.read_from(s);
}



