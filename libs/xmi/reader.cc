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
  model(0),
  xmi_version(0),
  uml_version(0)
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
// Translate XMI 1.0 fully qualified elements into XMI 1.1+ namespace ones
void Reader::upgrade_to_1_1(XML::Element &root)
{
  map<string, string> upgrade;

  // Build map 1.0 -> 1.1
  // Note - we only map things we're interested in, leaving the document
  // a mixture of old and new names - beware, if you want to use the
  // XML document for things our model doesn't cover, and you want to 
  // read XMI 1.0.  One option is to do your own mapping of the document
  // like this before you process it.

  // Main element names
  upgrade["Model_Management.Model"]       = "UML:Model";
  upgrade["Model_Management.Package"]     = "UML:Package";
  upgrade["Foundation.Core.Class"]        = "UML:Class";
  upgrade["Foundation.Core.DataType"]     = "UML:DataType";
  upgrade["Foundation.Core.Stereotype"]   = "UML:Stereotype";
  upgrade["Foundation.Core.Attribute"]    = "UML:Attribute";
  upgrade["Foundation.Core.Operation"]    = "UML:Operation";
  upgrade["Foundation.Core.Parameter"]    = "UML:Parameter";
  upgrade["Foundation.Core.AssociationClass"]  = "UML:Association";
  upgrade["Foundation.Core.AssociationEnd"]    = "UML:AssociationEnd";
 
  // 'Property' names
  upgrade["Foundation.Core.ModelElement.name"] = "UML:ModelElement.name";
  upgrade["Foundation.Core.ModelElement.visibility"] 
    = "UML:ModelElement.visibility";
  upgrade["Foundation.Core.GeneralizableElement.isAbstract"]
    = "UML:GeneralizableElement.isAbstract";
  upgrade["Foundation.Core.StructuralFeature.type"]
    = "UML:StructuralFeature.type";
  upgrade["Foundation.Core.Classifier"] = "UML:Classifier";

  upgrade["Foundation.Data_Types.Multiplicity"] = "UML:Multiplicity";
  upgrade["Foundation.Data_Types.MultiplicityRange"] = "UML:MultiplicityRange";
  upgrade["Foundation.Data_Types.MultiplicityRange.lower"] = 
    "UML:MultiplicityRange.lower";
  upgrade["Foundation.Data_Types.MultiplicityRange.upper"] = 
    "UML:MultiplicityRange.upper";

  // Translate
  root.translate(upgrade);

  cout << root;
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

  //Capture XMI version
  xmi_version = atof(root.get_attr("xmi.version").c_str());

  //See if we can find XMI.header/XMI.metamodel
  XML::Element& xmi_header = root.get_child("XMI.header");
  if (xmi_header.valid())
  {
    XML::Element& xmi_metamodel = xmi_header.get_child("XMI.metamodel");
    if (xmi_metamodel.valid())
    {
      if (xmi_metamodel.get_attr("xmi.name")=="UML")
	uml_version = atof(xmi_metamodel.get_attr("xmi.version").c_str());
      else
	warning("XMI.metamodel claims this isn't UML");
    }
  }

  //Get XMI.content
  XML::Element& xmi_content = root.get_child("XMI.content");
  if (!xmi_content.valid()) error("No <XMI.content> in <XMI>");

  //Before delving into UML, check for 1.0 and upgrade to 1.1 names
  //Do this even if version unknown - can't do any harm
  if (xmi_version < 1.1) upgrade_to_1_1(root);

  //Get UML model - assume only one
  XML::Element& modele = xmi_content.get_child("UML:Model");
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



