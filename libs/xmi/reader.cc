//==========================================================================
// ObTools::XMI: reader.cc
//
// XMI reader class
//
// Copyright (c) 2003 xMill Consulting Limited.  All rights reserved
// @@@ MASTER SOURCE - PROPRIETARY AND CONFIDENTIAL - NO LICENCE GRANTED
//==========================================================================

#include "ot-xmi.h"
#include <stdlib.h>

using namespace ObTools::XMI;
using namespace ObTools; 

//--------------------------------------------------------------------------
// Constructor
Reader::Reader(ostream&s):
  serr(s),
  xml_parser(s),
  model(0),
  xmi_version(0)
{
  // Add UML namespaces to parser (seen both of these!)
  xml_parser.fix_namespace("org.omg.xmi.namespace.UML", "UML");
  xml_parser.fix_namespace("org.omg/UML1.3", "UML");
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
//Record an ID to UML element mappimg
void Reader::record_uml_element(const string& id, UML::Element *e)
{
  uml_element_map[id]=e;
}

//------------------------------------------------------------------------
// Lookup an element in the UML element map by ID
// Returns 0 if failed
UML::Element *Reader::lookup_uml_element(const string& id)
{
  map<string,UML::Element *>::iterator p=uml_element_map.find(id);
  if (p!=uml_element_map.end()) return p->second;

  warning("Bad UML element reference idref ", id);
  return 0;
}

//------------------------------------------------------------------------
// Gather XML element IDs from this element, and recurse
void Reader::gather_xml_element_ids(XML::Element& e)
{
  //Get id 
  string id = e["xmi.id"];

  if (!id.empty())
    xml_element_map[id] = &e;

  // Recurse
  for(XML::Element::iterator p(e.children); p; ++p)
    gather_xml_element_ids(*p);
}

//------------------------------------------------------------------------
// Lookup an element in the XML element map by ID
// Returns 0 if failed
XML::Element *Reader::lookup_xml_element(const string& id)
{
  map<string,XML::Element *>::iterator p=xml_element_map.find(id);
  if (p!=xml_element_map.end()) return p->second;

  warning("Bad XML element reference idref ", id);
  return 0;
}

//------------------------------------------------------------------------
// Translate XMI 1.0 fully qualified elements into XMI 1.1+ namespace ones
void Reader::upgrade_xmi_to_1_1(XML::Element &root)
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
  upgrade["Foundation.Core.Enumeration"]  = "UML:Enumeration";
  upgrade["Foundation.Core.EnumerationLiteral"]  = 
          "UML:EnumerationLiteral";
  upgrade["Foundation.Core.Primitive"]    = "UML:Primitive";
  upgrade["Foundation.Core.Stereotype"]   = "UML:Stereotype";
  upgrade["Foundation.Core.Attribute"]    = "UML:Attribute";
  upgrade["Foundation.Core.Operation"]    = "UML:Operation";
  upgrade["Foundation.Core.Parameter"]    = "UML:Parameter";
  upgrade["Foundation.Core.Association"]  = "UML:Association";
  upgrade["Foundation.Core.AssociationClass"]  = "UML:AssociationClass";
  upgrade["Foundation.Core.AssociationEnd"]    = "UML:AssociationEnd";
  upgrade["Foundation.Core.Classifier"]        = "UML:Classifier";
  upgrade["Foundation.Core.Generalization"]    = "UML:Generalization";
  upgrade["Foundation.Core.GeneralizableElement"] = "UML:GeneralizableElement";

  // 'Property' names
  upgrade["Foundation.Core.ModelElement.name"] = "UML:ModelElement.name";
  upgrade["Foundation.Core.ModelElement.visibility"] = 
          "UML:ModelElement.visibility";
  upgrade["Foundation.Core.ModelElement.stereotype"] = 
          "UML:ModelElement.stereotype";

  upgrade["Foundation.Core.GeneralizableElement.isAbstract"] = 
          "UML:GeneralizableElement.isAbstract";
  upgrade["Foundation.Core.GeneralizableElement.isRoot"] = 
          "UML:GeneralizableElement.isRoot";
  upgrade["Foundation.Core.GeneralizableElement.isLeaf"] = 
          "UML:GeneralizableElement.isLeaf";

  upgrade["Foundation.Core.Generalization.parent"] = 
          "UML:Generalization.parent";
  upgrade["Foundation.Core.Generalization.child"] = 
          "UML:Generalization.child";

  upgrade["Foundation.Core.Class.isActive"] = "UML:Class.isActive";

  upgrade["Foundation.Core.AssociationEnd.ordering"] =
          "UML:AssociationEnd.ordering";
  upgrade["Foundation.Core.AssociationEnd.aggregation"] =
          "UML:AssociationEnd.aggregation";
  upgrade["Foundation.Core.AssociationEnd.multiplicity"] =
          "UML:AssociationEnd.multiplicity";
  upgrade["Foundation.Core.AssociationEnd.isNavigable"] =
          "UML:AssociationEnd.isNavigable";
  upgrade["Foundation.Core.AssociationEnd.participant"] =
          "UML:AssociationEnd.participant";
  //Also cope with UML1.3 'type' - conversion to 'participant' happens
  //in UML upgrade
  upgrade["Foundation.Core.AssociationEnd.type"] =
          "UML:AssociationEnd.type";

  upgrade["Foundation.Core.Feature.ownerScope"] = "UML:Feature.ownerScope";
  upgrade["Foundation.Core.StructuralFeature.type"] = 
          "UML:StructuralFeature.type";
  upgrade["Foundation.Core.StructuralFeature.ordering"] = 
          "UML:StructuralFeature.ordering";
  upgrade["Foundation.Core.Attribute.initialValue"] = 
          "UML:Attribute.initialValue";
  upgrade["Foundation.Core.BehaviouralFeature.isQuery"] = 
          "UML:BehaviouralFeature.isQuery";

  upgrade["Foundation.Core.Operation.isAbstract"] = "UML:Operation.isAbstract";
  upgrade["Foundation.Core.Operation.isRoot"] = "UML:Operation.isRoot";
  upgrade["Foundation.Core.Operation.isLeaf"] = "UML:Operation.isLeaf";
  upgrade["Foundation.Core.Operation.concurrency"] = 
          "UML:Operation.concurrency";
  upgrade["Foundation.Core.Parameter.kind"] = "UML:Parameter.kind";
  upgrade["Foundation.Core.Parameter.type"] = "UML:Parameter.type";

  upgrade["Foundation.Data_Types.Multiplicity"] = "UML:Multiplicity";
  upgrade["Foundation.Data_Types.MultiplicityRange"] = "UML:MultiplicityRange";
  upgrade["Foundation.Data_Types.MultiplicityRange.lower"] = 
    "UML:MultiplicityRange.lower";
  upgrade["Foundation.Data_Types.MultiplicityRange.upper"] = 
    "UML:MultiplicityRange.upper";

  upgrade["Foundation.Data_Types.Expression"] = "UML:Expression";
  upgrade["Foundation.Data_Types.Expression.language"] = 
          "UML:Expression.language";
  upgrade["Foundation.Data_Types.Expression.body"] = 
          "UML:Expression.body";

  // Translate
  root.translate(upgrade);
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
  double uml_version = 0;
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
  if (xmi_version < 1.1) upgrade_xmi_to_1_1(root);

  //Get UML model - assume only one
  XML::Element& modele = xmi_content.get_child("UML:Model");
  if (!modele.valid()) error("No <UML:Model> in <XMI.content>");

  // Read elements with an ID into XML element map
  gather_xml_element_ids(modele);

  //Now read model into a UML Model
  model = new UML::Model(*this, modele, uml_version);
}

//------------------------------------------------------------------------
// >> operator to read from istream
istream& ObTools::XMI::operator>>(istream& s, Reader& p) throw (ParseFailed)
{ 
  p.read_from(s);
  return s;
}

//--------------------------------------------------------------------------
// Destructor
Reader::~Reader()
{
  if (model) delete model;
}



